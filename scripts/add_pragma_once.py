#!/usr/bin/env python3
"""Add #pragma once to headers that lack any include guard."""
import os
import re

ROOTS = [
    'Minecraft.World',
    'Minecraft.Client',
    '4J.Render',
    '4J.Input',
    '4J.Profile',
    '4J.Storage',
]

GUARD_RE = re.compile(r'^\s*#\s*(pragma\s+once|ifndef\s+\w)', re.MULTILINE)

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
patched = 0

for root in ROOTS:
    for dirpath, _, files in os.walk(os.path.join(BASE, root)):
        for f in files:
            if not f.endswith('.h'):
                continue
            path = os.path.join(dirpath, f)
            try:
                content = open(path, 'r', encoding='utf-8', errors='replace').read()
            except Exception:
                continue
            if not GUARD_RE.search(content):
                open(path, 'w', encoding='utf-8').write('#pragma once\n' + content)
                patched += 1

print(f'Patched {patched} headers with #pragma once')
