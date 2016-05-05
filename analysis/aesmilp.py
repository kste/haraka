"""
A tool to find the minimum number of active S-boxes for AES-like ciphers and
Haraka-like designs. It can also be used to find the optimal truncated 
differential attack for Haraka.

The gurobi python interface is required to run this code http://www.gurobi.com/
"""

from argparse import ArgumentParser, RawTextHelpFormatter
from models import aeslike, haraka
from gurobipy import *

import yaml

# Disable logging for gurobi on console
setParam("LogToConsole", 1)

def activesboxharaka():
    config = {"rounds": 1,
              "wordsize": 8,
              "branchnumber": 5,
              "statedimension": 4,
              "aesstates": 4,
              "aesrounds": 2,
              "collisiononly": False,
              "mixlayer": "mix",
              "securitymodel": "sbox"}

    print("Rounds", "S-boxes", sep="\t")
    for num_rounds in range(1, 8):
        print(num_rounds, end='')
        for aes_rounds in range(1, 6):
            config["rounds"] = num_rounds
            config["aesrounds"] = aes_rounds
            solved_model = solvemodel(haraka.buildmodel(config))
            print(" ", round(solved_model.ObjVal), end="")
        print("")

def findminactiveincreasing():
    """
    Example for finding minimum active S-box for increasing number of
    rounds.
    """
    config = {"rounds": 1,
              "wordsize": 8,
              "branchnumber": 5,
              "statedimension": 4}

    print("Rounds", "S-boxes", sep="\t")
    for num_rounds in range(1, 11):
        config["rounds"] = num_rounds
        solved_model = solvemodel(aeslike.buildmodel(config))
        print(num_rounds, round(solved_model.ObjVal), sep="\t")
    return

def findminactivesbox(config):
    """
    Example which finds the minimum number of active S-boxes for AES like
    ciphers, with the parameters given in the config file.
    """
    if config["name"] == "aeslike":
        model = aeslike.buildmodel(config)
        solved_model = solvemodel(model)
        aeslike.printmodel(solved_model, config)
    elif config["name"] == "haraka":
        model = haraka.buildmodel(config)
        solved_model = solvemodel(model)
        haraka.printmodel(solved_model, config)
    return

def harakatruncated(config):
    """
    Find best attack in our truncated model for Haraka.
    """
    num_states = ((config["aesrounds"] + 1) * config["rounds"]) + 1

    # Iterate over all possible starting states for the attack
    best_attack = 999999
    best_round = -1
    best_model = 0

    for rnd in range(num_states - 1):
        if haraka.isAESround(rnd, config["aesrounds"]):
            config["attackerstart"] = rnd
            model = haraka.buildmodel(config)
            attack_costs = round(solvemodel(model).objVal)
            print("Subround {} - Best Attack: {}".format(rnd, attack_costs))
            if attack_costs < best_attack:
                best_attack = attack_costs
                best_round = rnd
                best_model = model

    print("Found best attack in round {} with costs {}".format(best_round,
                                                               best_attack))
    haraka.printmodel(best_model, config)
    return


def solvemodel(gurobi_model):
    """
    Solve model and return.
    """
    try:
        gurobi_model.update()
        gurobi_model.write('haraka.lp')
        gurobi_model.optimize()
    except GurobiError:
        print("Error when solving!")
        print(GurobiError)
    return gurobi_model

def main():
    """
    Load a config file and parse it
    """
    parser = ArgumentParser(description="todo",
                            formatter_class=RawTextHelpFormatter)
    parser.add_argument('--config', nargs=1, help="Use a yaml input file to"
                                                  "read the parameters")
    parser.add_argument('--sbox', action="store_true", 
                        help="Count the number of active S-boxes.")
    parser.add_argument('--truncated', action="store_true", 
                        help="Use the truncated model for security analysis.")
    parser.add_argument('--verb', nargs=1,
                        help="Set verbosity of the Gurobi solver.")
    args = parser.parse_args()

    params = {}

    #activesboxharaka()

    if args.verb:
        setParam("LogToConsole", int(args.verb[0]))

    if args.config:
        with open(args.config[0], 'r') as config:
            params = yaml.load(config)

    if args.sbox:
        findminactivesbox(params)
    
    if args.truncated:
        harakatruncated(params)


if __name__ == '__main__':
    main()
