#!/usr/bin/env python3
"""Reproduce one mathematical witness; failures never become proof receipts."""
from __future__ import annotations
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
WORK = ROOT / '_' / 'work'
RECEIPTS = ROOT / 'receipts'
IDRIC_REVISION = '94dfd99bd3e376507fedc8611053b7173b2519f0'
LETTERS = dict(zip('aAbB', ('over_first', 'under_first', 'over_second', 'under_second')))
ORDERS = ('012', '021', '102', '120', '201', '210')
WINDINGS = ('zero', 'one', 'two')


def run(name: str, command: list[str], cwd: Path, *, rejects: tuple[str, ...] = ()) -> str:
    result = subprocess.run(command, cwd=cwd, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=300)
    RECEIPTS.mkdir(parents=True, exist_ok=True)
    (RECEIPTS / f'{name}.log').write_text(result.stdout, encoding='utf-8')
    (RECEIPTS / f'{name}.json').write_text(json.dumps({
        'command': command, 'cwd': str(cwd), 'exit_code': result.returncode,
        'expected_rejection': bool(rejects)}, indent=2) + '\n', encoding='utf-8')
    print(result.stdout, end='', flush=True)
    if rejects:
        if result.returncode == 0 or not any(token in result.stdout for token in rejects):
            raise RuntimeError(f'{name}: required type-error rejection not observed')
    elif result.returncode != 0:
        raise RuntimeError(f'{name}: exited {result.returncode}')
    return result.stdout


def project(word: str, before: tuple[int, ...] = (0, 1, 2)) -> str:
    order = list(before)
    for letter in word:
        place = 0 if letter.lower() == 'a' else 1
        order[place], order[place + 1] = order[place + 1], order[place]
    return ''.join(map(str, order))


def winding(word: str) -> int:
    return sum(1 if letter.islower() else -1 for letter in word) % 3


def fixtures() -> list[dict]:
    rows = json.loads((ROOT / 'fixtures.json').read_text(encoding='utf-8'))
    assert len({row['name'] for row in rows}) == len(rows)
    for row in rows:
        assert set(row['word']) <= set(LETTERS)
        assert row['permutation'] == project(row['word']), row
        assert row['winding_mod_three'] == winding(row['word']), row
    return rows


def reference() -> None:
    rows = fixtures()
    checked = 0
    for length in range(7):
        for letters in itertools.product(LETTERS, repeat=length):
            word = ''.join(letters)
            assert project(word) in ORDERS
            for seed in itertools.permutations((0, 1, 2)):
                for generator in LETTERS:
                    assert project(generator + generator.swapcase() + word, seed) == project(word, seed)
                assert project('aba' + word, seed) == project('bab' + word, seed)
            for generator in LETTERS:
                assert winding(generator + generator.swapcase() + word) == winding(word)
            assert winding('aba' + word) == winding('bab' + word)
            checked += 1
    assert project('aa') == project('') and winding('aa') != winding('')
    # These kill two plausible bad implementations, but are not compiler tests.
    assert len('aA') % 3 != winding('aA')  # forgot crossing signs
    assert project('') != project('a')   # every crossing became a no-op
    print(json.dumps({'boundary': 'finite Python reference, NOT language proof',
                      'words': checked, 'shared_fixtures': len(rows)}, indent=2))


def prepare(lane: str) -> Path:
    work = WORK / lane
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    extension = 'idric' if lane == 'idric' else 'agda'
    shutil.copy2(ROOT / lane / f'BraidProjection.{extension}', work)
    rows = fixtures()
    if lane == 'idric':
        lines = ['module FixtureProofs', 'import BraidProjection', 'import Data.Text', '']
        lines += ['order_code : permutation → Text']
        lines += [f'order_code order_{order} = "{order}"' for order in ORDERS]
        lines += ['winding_code : winding_mod_three → Text']
        lines += [f'winding_code winding_{name} = "{value}"' for value, name in enumerate(WINDINGS)]
    else:
        lines = ['{-# OPTIONS --safe --without-K #-}', 'module FixtureProofs where',
                 'open import BraidProjection', 'open import Agda.Builtin.Equality using (_≡_; refl)', '']
    emitted = []
    for index, row in enumerate(rows):
        if lane == 'idric':
            word = '[' + ', '.join(LETTERS[x] for x in row['word']) + ']'
            lines += [f'endpoints_{index} : project_endpoints {word} = order_{row["permutation"]}',
                      f'endpoints_{index} = Refl',
                      f'winding_{index} : winding {word} = winding_{WINDINGS[row["winding_mod_three"]]}',
                      f'winding_{index} = Refl', '']
            emitted += [f'  putStrLn ("{row["name"]}\\t" ++ order_code (project_endpoints {word}) ++ "\\t" ++ winding_code (winding {word}))']
        else:
            word = '(' + ''.join(LETTERS[x].replace('_', '-') + ' ∷ ' for x in row['word']) + 'empty)'
            lines += [f'endpoints-{index} : project {word} ≡ order{row["permutation"]}',
                      f'endpoints-{index} = refl',
                      f'winding-{index} : winding {word} ≡ {WINDINGS[row["winding_mod_three"]]}',
                      f'winding-{index} = refl', '']
    if lane == 'idric':
        lines += ['main : IO ()', 'main = do'] + emitted
        (work / 'braid-fixtures.ipkg').write_text(
            'package braid-fixtures\nmodules = BraidProjection, FixtureProofs\n'
            'main = FixtureProofs\nexecutable = braid-fixtures\n', encoding='utf-8')
    (work / f'FixtureProofs.{extension}').write_text('\n'.join(lines) + '\n', encoding='utf-8')
    return work


def check_lane(lane: str) -> None:
    work = prepare(lane)
    if lane == 'agda':
        version = run('agda-version', ['agda', '--version'], work).strip()
        if version != 'Agda version 2.6.3':
            raise RuntimeError(f'Wrong Agda version: {version}')
        checker = ['agda', '--safe', '--without-K', '--no-libraries', '-i', '.']
        run('agda-positive', checker + ['FixtureProofs.agda'], work)
        bad = {
            'FalseWinding': ('bad : winding (over-first ∷ over-first ∷ empty) ≡ zero\nbad = refl\n', ('two', 'zero')),
            'ConfusedCarrier': ('bad : Permutation\nbad = over-first\n', ('Crossing', 'Permutation')),
            'WordEquality': ('bad : (over-first ∷ over-second ∷ over-first ∷ empty) ≡ (over-second ∷ over-first ∷ over-second ∷ empty)\nbad = refl\n', ('over-first', 'over-second')),
            'FalseBraidIdentity': ('bad : (over-first ∷ over-first ∷ empty) ≈ empty\nbad = same empty\n', ('empty', 'over-first')),
        }
        for name, (body, tokens) in bad.items():
            (work / f'{name}.agda').write_text('{-# OPTIONS --safe --without-K #-}\nmodule ' + name +
                ' where\nopen import BraidProjection\nopen import Agda.Builtin.Equality using (_≡_; refl)\n' + body, encoding='utf-8')
            run('agda-reject-' + name, checker + [name + '.agda'], work, rejects=tokens)
        old, new = 'first-swap order012 = order102', 'first-swap order012 = order012'
        extension, tokens = 'agda', ('order012', 'order102')
    else:
        checkout = Path(os.environ['IDRIC_CHECKOUT']).resolve()
        actual = run('idric-revision', ['git', 'rev-parse', 'HEAD'], checkout).strip()
        if actual != IDRIC_REVISION:
            raise RuntimeError(f'Wrong Idriç revision: {actual}')
        run('idric-clean-source', ['git', 'diff', '--exit-code', 'HEAD', '--'], checkout)
        compiler = checkout / '_' / 'build' / 'exec' / 'idris2'
        if not compiler.is_file():
            raise RuntimeError('Pinned Idriç compiler has not been bootstrapped')
        libs = checkout / '_' / 'libs'
        os.environ['IDRIS2_PATH'] = ':'.join(str(libs / name / 'build' / 'ttc') for name in ('prelude', 'base', 'linear'))
        os.environ['IDRIS2_LIBS'] = str(checkout / '_' / 'support' / 'c')
        os.environ['LD_LIBRARY_PATH'] = os.environ['IDRIS2_LIBS'] + ':' + os.environ.get('LD_LIBRARY_PATH', '')
        os.environ['PATH'] = str(checkout / '.tools' / 'bin') + ':' + os.environ['PATH']
        run('idric-version', [str(compiler), '--version'], work)
        run('idric-positive', [str(compiler), '--cg', 'chez', '--build', 'braid-fixtures.ipkg'], work)
        actual_output = run('idric-execution', [str(work / 'build' / 'exec' / 'braid-fixtures')], work)
        expected_output = ''.join(f'{r["name"]}\t{r["permutation"]}\t{r["winding_mod_three"]}\n' for r in fixtures())
        if actual_output != expected_output:
            raise RuntimeError('Idriç executable output disagrees with shared fixtures')
        checker = [str(compiler), '--check']
        bad = {
            'FalseWinding': ('bad : winding [over_first, over_first] = winding_zero\nbad = Refl\n', ('winding_two', 'winding_zero')),
            'ConfusedCarrier': ('bad : permutation\nbad = over_first\n', ('crossing', 'permutation')),
            'WordEquality': ('bad : [over_first, over_second, over_first] = [over_second, over_first, over_second]\nbad = Refl\n', ('over_first', 'over_second')),
            'FalseBraidIdentity': ('bad : BraidEquivalent [over_first, over_first] []\nbad = SameWord []\n', ('over_first', 'BraidEquivalent')),
        }
        for name, (body, tokens) in bad.items():
            (work / f'{name}.idric').write_text('module ' + name + '\nimport BraidProjection\n' + body, encoding='utf-8')
            run('idric-reject-' + name, checker + [name + '.idric'], work, rejects=tokens)
        old, new = 'swap_first order_012 = order_102', 'swap_first order_012 = order_012'
        extension, tokens = 'idric', ('first_twice', 'order_102')
    mutant = WORK / (lane + '-mutant')
    if mutant.exists():
        shutil.rmtree(mutant)
    mutant.mkdir(parents=True)
    source = (work / f'BraidProjection.{extension}').read_text(encoding='utf-8')
    assert source.count(old) == 1
    (mutant / f'BraidProjection.{extension}').write_text(source.replace(old, new), encoding='utf-8')
    run(lane + '-reject-mutant', checker + ['BraidProjection.' + extension], mutant, rejects=tokens)
    revision = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    inputs = [ROOT / 'idric' / 'BraidProjection.idric', ROOT / 'agda' / 'BraidProjection.agda',
              ROOT / 'fixtures.json', Path(__file__).resolve()]
    receipt = {'lane': lane, 'source_revision': revision, 'status': 'CHECKED',
               'boundary': 'host Chez execution and typechecking' if lane == 'idric' else 'safe typechecking, no executable backend',
               'rejected_programs': len(bad), 'rejected_core_mutants': 1,
               'sha256': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}}
    (RECEIPTS / (lane + '-summary.json')).write_text(json.dumps(receipt, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(receipt, indent=2))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('lane', choices=('reference', 'agda', 'idric'))
    args = parser.parse_args()
    if args.lane == 'reference':
        reference()
    else:
        check_lane(args.lane)


if __name__ == '__main__':
    main()
