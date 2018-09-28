#!/usr/bin/env python

'''Condenses the error policy table down to only the fields used by
   the BMC Code.

This script pulls the following 4 fields out of the full JSON policy
table and arranges them for easier searching:

    Error:         The OpenBMC error.
                   e.g. xyz.openbmc_project.Thermal.Error.PowerSupplyHot
    CommonEventID: Used to index into the online documentation.
                   e.g. FQPSPCA0065M
    Modifier:      Used in combination with the error to locate a
                   table entry.
                   e.g. <an inventory callout>
    Message:       A short message describing the error.
                   e.g. "Power supply 0 is too hot"

There may be multiple CommonEventID/Modifier/Message groups per Error,
which is why both the error and modifier are needed to find an entry.

Example condensed entry, prettified:
    {
      "dtls":[
        {
          "CEID":"FQPSPCA0065M",
          "mod":"/xyz/openbmc_project/inventory/system/ps0",
          "msg":"Power supply 0 is too hot"
        },
        {
          "CEID":"FQPSPCA0066M",
          "mod":"/xyz/openbmc_project/inventory/system/ps1",
          "msg":"Power supply 1 is too hot"
        }
      ],
      "err":"xyz.openbmc_project.Thermal.Error.PowerSupplyHot"
    }
'''

import argparse
import sys
import json


def add_details(error, details, condensed):
    '''Adds a details entry to an error'''

    found = False
    for errors in condensed:
        if errors['err'] == error:
            errors['dtls'].append(details)
            found = True
            break

    if not found:
        group = {}
        group['err'] = error
        group['dtls'] = []
        group['dtls'].append(details)
        condensed.append(group)


if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Error log policy condenser')

    parser.add_argument('-p', '--policy',
                        dest='policy_file',
                        default='policyTable.json',
                        help='Policy Table in JSON')
    parser.add_argument('-c', '--condensed_policy',
                        dest='condensed_file',
                        default='condensed.json',
                        help='Condensed policy output file in JSON')
    parser.add_argument('-t', '--prettify_json',
                        dest='prettify',
                        default=False,
                        action='store_true',
                        help='Prettify the output JSON')

    args = parser.parse_args()

    with open(args.policy_file, 'r') as table:
        contents = json.load(table)
        policytable = contents['events']

    condensed = []

    for name in policytable:
        details = {}

        #Parse the error||modifer line. The modifier is optional.
        separatorPos = name.find('||')
        if separatorPos != -1:
            error = name[0:separatorPos]
            modifier = name[separatorPos + 2:]
            details['mod'] = modifier
        else:
            error = name
            details['mod'] = ''

        #The table has some nonBMC errors - they have spaces - skip them
        if ' ' in error:
            print("Skipping error %s because of spaces" % error)
            continue

        details['msg'] = policytable[name]['Message']
        details['CEID'] = policytable[name]['CommonEventID']

        add_details(error, details, condensed)

    #if prettified there will be newlines
    indent_value = 2 if args.prettify else None

    with open(args.condensed_file, 'w') as outfile:
        json.dump(condensed, outfile, separators=(',', ':'),
                  indent=indent_value)
