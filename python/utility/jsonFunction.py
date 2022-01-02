#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json

def readJson(filename, json_key=''):
  try:
    json_data = json.load(open(filename, 'r'))

  except FileNotFoundError:
    return filename + ": not found."

  except PermissionError:
    return filename + ": permission error."

  else:
    if bool(json_key and json_key.strip()):
      return json_data[json_key]

    else:
      return json_data

# Print out all the stuff in json_data.
#    for key in json_data:
#      value = json_data[key]
#      print("Key: {} | Value: {}".format(key, value))
