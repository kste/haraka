"""
This script constructs a MILP model for AES-like primitives, which can aid in
finding optimal parameter sets against differential attacks by counting the
minimum number of active S-boxes in a differential trail.

It uses the Gurobi Solver to solve the MILP instance, hence you need
a Gurobi license (free for Academic use).
"""

from gurobipy import *
from models.milpconstraints import addAESrndconstraints


def buildmodel(config):
    """
    Constructs the model for the Gurobi Solver
    """

    model = Model("aeslike")

    # Parameters
    state_dim = config["statedimension"]
    num_rounds = config["rounds"]
    branch_number = config["branchnumber"]

    # Initialize all variables
    var_x = [] # state
    var_d = [] # dummy variable for MixColumns

    state_words = state_dim * state_dim

    for byte in range((num_rounds + 1) * state_words):
        var_x.append(model.addVar(vtype=GRB.BINARY, name="x[{}]".format(byte)))
    for col in range(num_rounds * state_dim):
        var_d.append(model.addVar(name="dummy[{}]".format(col)))

    activesboxes = model.addVar(name="Active S-boxes")

    model.update()

    # Constraints

    # Optimize number of active S-boxes
    model.setObjective(activesboxes, GRB.MINIMIZE)

    # Count Active S-boxes
    model.addConstr(quicksum(var_x[i] for i in range(num_rounds * state_words))
                           - activesboxes == 0, "Count Active S-boxes")

    # Add constraints from AES round function
    model = addAESrndconstraints(model, state_dim, var_x, var_d,
                                        branch_number, num_rounds)

    # No Zero Characteristic
    model.addConstr(quicksum(var_x[i] for i in range((num_rounds + 1) *
                           state_words)) >= 1, "Avoid trivial solutions")

    return model

def printmodel(model, config):
    """
    Print the solution and the corresponding differential trail.
    """
    state_dim = config["statedimension"]
    num_rounds = config["rounds"]

    print("Rounds:", num_rounds)
    print("State dimension:", state_dim)
    print("Branch number:", config["branchnumber"])
    print("Minimum number of active S-boxes: {}".format(model.objVal))

    print("Best differential trail:")

    # Print differential trail
    # Print Header
    header = ""
    for rnd in range(num_rounds + 1):
        header += str(rnd) + " " * (2 * state_dim + 1 - len(str(rnd)))

    print(header)

    # Print State
    for row in range(state_dim):
        for rnd in range(num_rounds + 1):
            for col in range(state_dim):
                cur_index = row + col * state_dim + rnd * state_dim * state_dim
                if model.getVarByName("x[{}]".format(cur_index)).x > 0.0:
                    print("\033[91mx\033[0m", end=" ")
                else:
                    print(".", end=" ")
            print(" ", end="")
        print("")
    return model.objVal
