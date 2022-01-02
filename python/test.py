#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import glob
import importlib
import re
import sys

sys.path.append(r'./utility')

from utility.jsonFunction import readJson


# Import all modules.
#jsonData = readJson('filename.txt', 'module')
#
#for mod in glob.glob('module/*/init.py'):
#  mod = re.split('[/.]', mod)[1]

# Load base modules.
jsonData = readJson('test.conf', 'module')

for mod in jsonData:
  globals()[mod] = importlib.import_module('module.' + mod + '.init')
  globals()[mod].init()

