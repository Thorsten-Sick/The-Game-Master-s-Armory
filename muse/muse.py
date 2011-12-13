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

COMMANDS = []

entries = [] # {"id:,"text","flags"=[]}

def get_matches(already):
    res = []
    for a in entries:
        if len(set(already) - set(a["flags"]))  == 0:
            res.append(a)

    return res

def get_ideas(flags, number=5):
    """ Return a list of ideas

    flags: relevant flags
    number: max. Number of ideas
    """

    res = []
    data = get_matches(flags)
    for i in  data:
        res.append (i["text"])
    if len(res) <= number:
        return res
    else:
        random.shuffle(res)
        return res[:4]

def readdb():
    fh = open("test.txt")
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

if __name__== "__main__":
    readdb()
    readline.parse_and_bind("tab: complete")
    readline.set_completer(complete)
    a=""
    while a != "quit":
        a = raw_input('Enter section name: ')
        print ">>>" + a
        command = a.split(" ")
        ideas = get_ideas(command)
        for i in ideas:
            print i

