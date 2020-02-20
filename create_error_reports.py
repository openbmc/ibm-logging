#!/usr/bin/env python

'''Generates 2 reports about OpenBMC error logs:

    1) Dumps every error defined in the errors.yaml files passed in
       into a single JSON file that looks like:

       {
         "desc":"Callout IIC device",
         "error":"xyz.openbmc_project.Common.Callout.Error.IIC",
         "file":"xyz/openbmc_project/Common/Callout.errors.yaml",
         "metadata":[
           "CALLOUT_IIC_BUS",
           "CALLOUT_IIC_ADDR",
           "Inherits xyz.openbmc_project.Common.Callout.Error.Device"
          ]
       }

    2) Crosschecks this generated JSON with the IBM error policy table,
       showing if any errors are in one file but not the other.

'''

import argparse
import os
import json
import yaml


def get_errors(yaml_dirs):
    '''Finds all of the errors in all of the error YAML files in
       the directories passed in.'''

    all_errors = []
    for yaml_dir in yaml_dirs:
        error_data = []
        yaml_files = get_yaml(yaml_dir)

        for yaml_file in yaml_files:
            all_errors += read_error_yaml(yaml_dir, yaml_file)

    return all_errors


def read_error_yaml(yaml_dir, yaml_file):
    '''Returns a list of dictionary objects reflecting the error YAML.'''

    all_errors = []

    #xyz/openbmc_project/x.errors.yaml -> xyz.openbmc_project.x.Error
    error_base = yaml_file.replace(os.sep, '.')
    error_base = error_base.replace('.errors.yaml', '')
    error_base += '.Error.'

    #Also needs to look up the metadata from the .metadata.yaml files
    metadata_file = yaml_file.replace('errors.yaml', 'metadata.yaml')
    metadata = []

    if os.path.exists(os.path.join(yaml_dir, metadata_file)):
        with open(os.path.join(yaml_dir, metadata_file)) as mfd:
            metadata = yaml.safe_load(mfd.read())

    with open(os.path.join(yaml_dir, yaml_file)) as fd:
        data = yaml.safe_load(fd.read())

        for e in data:
            error = {}
            error['error'] = error_base + e['name']
            error['desc'] = e['description']
            error['metadata'] = get_metadata(e['name'], metadata)
            error['file'] = yaml_file
            all_errors.append(error)

    return all_errors


def add_error(val):
    '''Adds the '.Error' before the last segment of an error string.'''
    dot = val.rfind('.')
    return val[:dot] + '.Error' + val[dot:]


def get_metadata(name, metadata):
    '''Finds metadata entries for the error in the metadata
       dictionary parsed out of the *.metadata.yaml files.

       The metadata YAML looks something like:
            - name: SlaveDetectionFailure
              meta:
                - str: "ERRNO=%d"
                  type: int32
              inherits:
                - xyz.openbmc_project.Callout
      '''

    data = []
    for m in metadata:
        if m['name'] == name:
            if 'meta' in m:
                for entry in m['meta']:
                    #Get the name from name=value
                    n = entry['str'].split('=')[0]
                    data.append(n)

            #inherits is a list, return it comma separated
            if 'inherits' in m:
                vals = list(map(add_error, m['inherits']))
                i = ','.join(vals)
                data.append("Inherits %s" % i)

    return data


def get_yaml(yaml_dir):
    '''Finds all of the *.errors.yaml files in the directory passed in.
       Returns a list of entries like xyz/openbmc_project/Common.Errors.yaml.
    '''

    err_files = []
    metadata_files = []
    if os.path.exists(yaml_dir):
        for directory, _, files in os.walk(yaml_dir):
            if not files:
                continue

            err_files += [os.path.relpath(
                    os.path.join(directory, f), yaml_dir)
                    for f in [f for f in files if f.endswith('.errors.yaml')]]

    return err_files


def crosscheck(errors, policy, outfile):
    '''Crosschecks that the errors found in the YAML are in the
       policy file, and vice versa.
    '''

    policy_errors = [x['err'] for x in policy]
    yaml_errors = [x['error'] for x in errors]

    out = open(outfile, 'w')
    out.write("YAML errors not in policy table:\n\n")

    for e in yaml_errors:
        if e not in policy_errors:
            out.write("    %s\n" % e)

    out.write("\n%d total errors in the YAML\n\n" % len(yaml_errors))
    out.write("Policy errors not in YAML:\n\n")

    for e in policy_errors:
        if e not in yaml_errors:
            out.write("    %s\n" % e)

    num_details = 0
    for e in policy:
        for d in e['dtls']:
            num_details += 1

    out.write("\n%d total errors (with %d total details blocks) in the "
              "policy table\n\n" % (len(policy_errors), num_details))

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Error log policy reports')

    parser.add_argument('-y', '--yaml_dirs',
                        dest='yaml_dirs',
                        default='.',
                        help='Comma separated list of error YAML dirs')
    parser.add_argument('-e', '--error_file',
                        dest='error_file',
                        default='obmc-errors.json',
                        help='Output Error report file')
    parser.add_argument('-p', '--policy',
                        dest='policy_file',
                        default='condensed.json',
                        help='Condensed policy in JSON')
    parser.add_argument('-x', '--crosscheck',
                        dest='crosscheck_file',
                        help='YAML vs policy table crosscheck output file')

    args = parser.parse_args()

    dirs = args.yaml_dirs.split(',')
    errors = get_errors(dirs)

    with open(args.error_file, 'w') as outfile:
        json.dump(errors, outfile, sort_keys=True,
                  indent=2, separators=(',', ':'))

    if args.crosscheck_file:
        with open(args.policy_file) as pf:
            policy = yaml.safe_load(pf.read())

        crosscheck(errors, policy, args.crosscheck_file)
