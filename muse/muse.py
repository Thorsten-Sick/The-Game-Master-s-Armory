#!/usr/bin/env python
# muse.py
#
# Copyright (C) 2011 - Thorsten Sick
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

"""
Muse offers a simple way to get inspiration from database files

"""


# http://stackoverflow.com/questions/2046050/tab-completion-in-python-command-line-interface-how-to-catch-tab-events

import readline
import argparse
import random

COMMANDS = []

entries = [] # {"id:,"text","flags"=[]}

def get_matches(already):
    res = []
    for a in entries:
        if len(set(already) - set(a["flags"]))  == 0:
            res.append(a)

    return res

def print_idealist(ilist, total):
    """ Prints a idea list
    """

    for i in ilist:
        print "\t"+i.strip()

    print "Total ideas: " + str(total)
        
    
def print_ideas(flags, number=5):
    """ Return a list of ideas

    flags: relevant flags
    number: max. Number of ideas
    """

    res = []
    data = get_matches(flags)
    for i in  data:
        res.append (i["text"])
    if len(res) <= number:
        print_idealist(res, len(res))
    else:
        random.shuffle(res)
        print_idealist(res[:number], len(res))

def readdb(filename):
    fh = open(filename)
    i = 0
    for aline in fh.readlines():
        entry = {}
        entry["id"] = i
        a,b = aline.split(":")
        entry["text"] = b
        entry["flags"] = []
        for item in a.split("/"):
            item = item.lower()
            entry["flags"].append(item)
            COMMANDS.append(item)
        entries.append(entry)
        i = i + 1

def getcommands(already):
    """
    """

    return COMMANDS
    """
    matches = get_matches(already)
    
    res = []    
    for a in matches:
        res = res + a["flags"]
    
    res = list(set(res))    
    print res

    return res
    """

def complete(text, state):
    already = []
    already = readline.get_line_buffer().split(" ")
    commands = getcommands(already)
    for cmd in commands:
        if cmd.startswith(text):
            if not state:
                return cmd
            else:
                state -= 1

def mainloop(databases, number = 5, cmdline = None):
    """ Main processing loop

    interactive: Stay in the loop till the user enters "quit"
    databases: a list of database files to use
    number: Number of items to display
    """

    for i in databases:
        readdb(i)
    readline.parse_and_bind("tab: complete")
    readline.set_completer(complete)

    if not cmdline:
        a=""
        while a != "quit":
            a = raw_input('Enter section name: ')
            print ">>>" + a
            command = a.split(" ")
            print_ideas(command, number)
            print "enter <quit> to quit"
    else:
        command = cmdline.split(",")
        print_ideas(command, number)

if __name__== "__main__":
    parser = argparse.ArgumentParser(description='Muse. Inspiration for the Game Master')
    parser.add_argument('--command', default=None, help="One shot command to parse. flags separated by ','")
    parser.add_argument('--number', type=int, default=5, help="Number of ideas per request")
    parser.add_argument('--databases', nargs='+', help='The database files to use')
                       
    args = parser.parse_args()

    mainloop(args.databases, args.number, args.command)
    

