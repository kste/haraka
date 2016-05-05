"""
This script constructs a MILP model for Haraka-like designs to
count the number of active S-boxes and determine the security
level against truncated differential attacks.

It uses the Gurobi Solver to solve the MILP instance, hence you need
a Gurobi license (free for Academic use).
"""

from gurobipy import *
from models.milpconstraints import addAESrndconstraints


def buildmodel(config):
    """
    Constructs the model for the Gurobi Solver
    """

    model = Model("haraka")

    # Parameters
    rounds = config["rounds"]
    state_dim = config["statedimension"]
    branch_number = config["branchnumber"]
    aes_rounds = config["aesrounds"]
    aes_states = config["aesstates"]

    num_states = ((aes_rounds + 1) * rounds) + 1
    words_state = state_dim * state_dim

    # Initialize all variables
    var_x = [[] for _ in range(aes_states)]
    var_d = [[] for _ in range(aes_states)]
    var_mccosts = [[] for _ in range(aes_states)]
    var_mcactive = [[] for _ in range(aes_states)]

    for aes_state in range(aes_states):
        for word in range(num_states * words_state):
            var_x[aes_state].append( 
                model.addVar(vtype=GRB.BINARY,
                             name="x[{}][{}]".format(aes_state, word))
                )
        for col in range(num_states * state_dim):
            var_d[aes_state].append(
                model.addVar(name="dummy[{}][{}]".format(aes_state, col))
                )
            var_mccosts[aes_state].append(
                model.addVar(name="MCCosts[{}][{}]".format(aes_state, col))
                )
            var_mcactive[aes_state].append(
                model.addVar(vtype=GRB.BINARY,
                             name="MCActive[{}][{}]".format(aes_state, col))
                )
    
    activesboxes = model.addVar(name="Active S-boxes")
    costs = model.addVar()


    model.update()

    # Objective to minimize attack costs
    model.setObjective(costs, GRB.MINIMIZE)

    if config["securitymodel"] == "sbox":
        # print("Finding minimum number of active S-boxes...")
        # Count number of active S-boxes
        model = addactivesboxconstraints(model, config, var_x, activesboxes)
        model.setObjective(activesboxes, GRB.MINIMIZE)
    elif config["securitymodel"] == "truncated":
        model = addtruncatedconstraints(model, config, var_x, var_mccosts,
                                        var_mcactive, costs)


    if config["collisiononly"]:
        if aes_states == 4:
            # If we have 4 states truncated to 256-bit
            model = addcolltruncoutput512(model, config, var_x)
        else:
            model = addcollisionconstraints(model, config, var_x)


    for rnd in range(rounds):
        # Add AES round constraints
        for aes_state in range(aes_states):
            model = addAESrndconstraints(model, state_dim, 
                var_x[aes_state][words_state * (aes_rounds + 1) * rnd:], 
                var_d[aes_state][state_dim * (aes_rounds + 1) * rnd:], 
                branch_number, aes_rounds)

        # Add MIX round constraints
        if config["mixlayer"] == "mix" and aes_states == 4:
            model = addmixconstraints512(model, config, var_x, rnd)
        elif config["mixlayer"] == "mix" and aes_states == 2:
            model = addmixconstraints256(model, config, var_x, rnd)


    # No all Zero
    model.addConstr(quicksum(var_x[aes_state][i] 
                             for aes_state in range(aes_states) 
                             for i in range((aes_rounds * rounds + 1) *
                                            state_dim * state_dim)) >= 1,
                           "notrivialsolution")

    return model


def filterAESround(rounds, aes_rounds):
    """
    Filters the list for AES rounds.
    """
    return filter(lambda x: isAESround(x, config["aesrounds"]), rounds)

def isAESround(rnd, aes_rounds):
    """
    Return True if rnd is an AES round.
    """
    return rnd == 0 or (((rnd + 1) % (aes_rounds + 1)) != 0)

def printmodel(model, config):
    """
    Print the solution and the corresponding differential trail.
    """
    if config["securitymodel"] == "truncated":
        print("MixColumns Costs: {}".format(round(model.getVarByName("MixColumnsCosts").x)))
        print("MixColumns Costs (no dof): {}".format(round(model.getVarByName("MixColumnsCostsNoDof").x)))
        print("Collision Costs: {}".format(round(model.getVarByName("CollisionCosts").x)))
        print("Reducable Costs: {}".format(round(model.getVarByName("ReducableCosts").x)))
        print("Degrees of Freedom: {}".format(round(model.getVarByName("DegreesOfFreedom").x)))

    print("Obj: {}".format(round(model.objVal)))

    print("Best differential trail:")

    state_dim = config["statedimension"]
    num_states = ((config["aesrounds"] + 1) * config["rounds"]) + 1

    for rnd in range(num_states):
        for row in range(state_dim):
            for aes_state in range(config["aesstates"]):
                for col in range(state_dim):
                    cur_index = row + col * state_dim + rnd * state_dim * state_dim
                    if model.getVarByName("x[{}][{}]".format(
                        aes_state, cur_index)).x > 0.5:
                        print("\033[91mx\033[0m", end = " ")
                    else:
                        print(".", end = " ")
                print(" ", end = "")
            print("")
        if rnd != num_states - 1:
            if isAESround(rnd, config["aesrounds"]):
                print("AES")
            else:
                print("MIX")
    return

def addtruncatedconstraints(model, config, var_x, var_mccosts, var_mcactive,
                            costs):
    """
    Adds constraints for the truncated security model.
    """

    costs_mc = model.addVar(name="MixColumnsCosts")
    costs_mc_nodof = model.addVar(name="MixColumnsCostsNoDof")
    costs_collision = model.addVar(name="CollisionCosts")
    costs_reducable = model.addVar(name="ReducableCosts")
    degoffree = model.addVar(name="DegreesOfFreedom")

    model.update()

    num_states = ((config["aesrounds"] + 1) * config["rounds"]) + 1
    state_dim = config["statedimension"]

    # Define costs

    # Attacker can control input == output difference with d.o.f.
    if config["attackerstart"] - config["attackerpower"] <= 0 and \
       config["attackerstart"] + config["attackerpower"] >= (num_states - 2):
       model.addConstr(costs_reducable >= costs_mc + costs_collision - 
                       degoffree, "Attack costs after reducing d.o.f.")
       model.addConstr(costs >= costs_reducable + costs_mc_nodof,
                       "Total attack costs")
    else:
        model.addConstr(costs_reducable >= costs_mc - degoffree, 
                        "Attack costs after reducing d.o.f.")
        model.addConstr(costs >= costs_reducable + costs_mc_nodof + 
                        costs_collision, "Total attack costs")

    # Count number of d.o.f.
    # Collision resistance
    # model.addConstr(degoffree <= state_dim * state_dim * config["wordsize"] * 
    #                config["aesstates"])

    # Second-preimage reistance
    # Allow only to choose differences in this setting
    start_indices = [config["attackerstart"] * state_dim * state_dim + 
                     x for x in range(state_dim*state_dim)]
    model.addConstr(degoffree <= quicksum(var_x[aes_state][i]
                        for aes_state in range(config["aesstates"])
                        for i in start_indices) * config["wordsize"])

    # Find rounds which are non-linear
    non_linear_rounds = [x for x in range(num_states - 1) if isAESround(x, 
                         config["aesrounds"])]

    start_index = non_linear_rounds.index(config["attackerstart"])

    # Count conditions on MixColumns
    for aes_state in range(config["aesstates"]):
        for fwd_rnd in non_linear_rounds[start_index:]:
            for col in range(state_dim):
                indices = []
                for row in range(state_dim):
                    indices.append((fwd_rnd + 1) * state_dim * state_dim +
                                col*state_dim + row)
                model = addMCcostsfromindices(model, config, var_x, var_mccosts,
                                              var_mcactive, aes_state, fwd_rnd, 
                                              col, indices)


        for bck_rnd in non_linear_rounds[:start_index]:
            for col in range(state_dim):
                indices = []
                for row in range(state_dim):
                    tmp_index = ((state_dim * col + row * (state_dim + 1)) %
                                 (state_dim * state_dim))
                    indices.append(bck_rnd * state_dim * state_dim + tmp_index)
                model = addMCcostsfromindices(model, config, var_x, var_mccosts,
                                              var_mcactive, aes_state, bck_rnd,
                                              col, indices)

    # Find costs for controlled and uncontrolled rounds
    assert config["attackerstart"] in non_linear_rounds
    match_index = non_linear_rounds.index(config["attackerstart"])

    dof_interval_from = max(match_index - config["attackerpower"], 0)
    dof_interval_to = min(match_index + config["attackerpower"], num_states)

    active_rounds_dof = non_linear_rounds[dof_interval_from:dof_interval_to]
    active_rounds_nodof = list(set(non_linear_rounds) - set(active_rounds_dof))

    mc_indices = []
    mc_indices_nodof = []

    for i in range(state_dim):
        for itrnd in active_rounds_dof:
            mc_indices.append(state_dim*itrnd + i)
        for itrnd in active_rounds_nodof:
            mc_indices_nodof.append(state_dim*itrnd + i)

    model.addConstr(quicksum(var_mccosts[j][i] for j in range(config["aesstates"]) 
        for i in mc_indices) - costs_mc == 0, "MixColumns Costs Reducable")
    model.addConstr(quicksum(var_mccosts[j][i] for j in range(config["aesstates"]) 
        for i in mc_indices_nodof) - costs_mc_nodof == 0, "MixColumns Costs")

    return model

def addMCcostsfromindices(model, config, var_x, var_mccosts, var_mcactive, 
                          aes_state, rnd, col, indices):
    """
    Add the MixColumns costs given the indices
    """
    state_column = [var_x[aes_state][i] for i in indices]
    column_idx = rnd * config["statedimension"] + col
    # Mark as active MixColumns
    model.addConstr(quicksum(state_column) <= config["statedimension"] * 
                    var_mcactive[aes_state][column_idx], 
                    "MixColumns is active")
    # Costs for MixColumn transition
    model.addConstr((config["statedimension"] - quicksum(state_column)) * 
                    config["wordsize"] * 
                    var_mcactive[aes_state][column_idx] == 
                    var_mccosts[aes_state][column_idx],
                    "MixColumns costs")
    return model

def addcolltruncoutput512(model, config, var_x):
    """
    Add constrains that the trail must lead to a collision after truncation.
    """
    assert(config["aesstates"] == 4)

    # haraka Truncation
    num_states = ((config["aesrounds"] + 1) * config["rounds"]) + 1
    state_dim = config["statedimension"]

    hashoutput = []
    for aes_state in [0, 3]:
        for word in range(2 * state_dim, state_dim * state_dim):
            hashoutput.append(var_x[aes_state][word])
            model.addConstr(var_x[aes_state][word] == 
                            var_x[aes_state][word + (num_states - 1) *
                            state_dim * state_dim], "collision")
    for aes_state in [1, 2]:
        for word in range(2 * state_dim):
            hashoutput.append(var_x[aes_state][word])
            model.addConstr(var_x[aes_state][word] == 
                            var_x[aes_state][word + (num_states - 1) *
                            state_dim * state_dim], "collision")            

    if config["securitymodel"] == "truncated":
        costs_collision = model.getVarByName("CollisionCosts")
        model.addConstr(costs_collision - quicksum(hashoutput) * 
                        config["wordsize"] == 0, "inputdiff = outputdiff")

    return model

def addcollisionconstraints(model, config, var_x):
    """
    Add constraints that the trail must lead to a collision.
    """
    num_states = ((config["aesrounds"] + 1) * config["rounds"]) + 1
    state_dim = config["statedimension"]

    for aes_state in range(config["aesstates"]):
        for word in range(state_dim * state_dim):
            model.addConstr(var_x[aes_state][word] == 
                            var_x[aes_state][word + (num_states - 1) * 
                            state_dim * state_dim], "collision")

    if config["securitymodel"] == "truncated":
        costs_collision = model.getVarByName("CollisionCosts")
        model.addConstr(costs_collision - 
                        quicksum(var_x[i][j] for i in range(config["aesstates"])
                                 for j in range(state_dim * state_dim)) * 
                        config["wordsize"] == 0, "inputdiff = outputdiff")            

    return model

def addmixconstraints512(model, config, var_x, current_round):
    """
    Adds the mix layer. Note that this layer is only defined if there
    are exactly four AES states.
    """
    assert(config["aesstates"] == 4)

    # Columnwise permutation
    permutation = [3, 11, 7, 15, 
                   8, 0, 12, 4, 
                   9, 1, 13, 5, 
                   2, 10, 6, 14]

    state_dim = config["statedimension"]
    words_state = state_dim * state_dim
    start_index = words_state * (config["aesrounds"] + current_round * 
                 config["aesrounds"] + current_round)

    next_index = 0

    for idx, col in enumerate(permutation):
        old_col_start = start_index + (col % state_dim) * state_dim
        new_col_start = start_index + (idx % state_dim) * state_dim + words_state
        for word in range(state_dim):
            model.addConstr(var_x[col // 4][old_col_start + word] ==
                            var_x[idx // 4][new_col_start + word], "mix")

    return model

def addmixconstraints256(model, config, var_x, current_round):
    """
    Adds the mix layer. Note that this layer is only defined if there
    are exactly two AES states.
    """
    assert(config["aesstates"] == 2)

    # Columnwise permutation
    permutation = [0, 4, 1, 5,
                   2, 6, 3, 7]

    state_dim = config["statedimension"]
    words_state = state_dim * state_dim
    start_index = words_state * (config["aesrounds"] + current_round * 
                 config["aesrounds"] + current_round)

    next_index = 0

    for idx, col in enumerate(permutation):
        old_col_start = start_index + (col % state_dim) * state_dim
        new_col_start = start_index + (idx % state_dim) * state_dim + words_state
        for word in range(state_dim):
            model.addConstr(var_x[col // 4][old_col_start + word] ==
                            var_x[idx // 4][new_col_start + word], "mix")

    return model    

def addactivesboxconstraints(model, config, var_x, activesboxes):
    """
    Adds constraints for counting the number of active S-boxes.
    """
    sbox_indices = []
    num_states = (config["aesrounds"] + 1) * config["rounds"]
    state_size = config["statedimension"] * config["statedimension"]
    for rnd in filter(lambda x: isAESround(x, config["aesrounds"]),
                      range(0, num_states)):
        words_state = config["statedimension"] * config["statedimension"]
        rnd_offset = rnd * words_state
        sbox_indices += [rnd_offset + word for word in range(words_state)]

    sboxes = []

    if config["aesstates"] == 4:
        # Remove S-boxes which are truncated
        trunc_indices = [0, 1, 5, 6, 10, 11, 12, 15]
        trunc_indices_2 = [2, 3, 4, 7, 8, 9, 13, 14]

        tmp_sbox_indices = [i for i in sbox_indices]
        for idx in trunc_indices:
            tmp_sbox_indices.remove(idx + (num_states - 2) * state_size)
        for idx in tmp_sbox_indices:
            sboxes.append(var_x[0][idx])
            sboxes.append(var_x[2][idx])

        tmp_sbox_indices = [i for i in sbox_indices]
        for idx in trunc_indices_2:
            tmp_sbox_indices.remove(idx + (num_states - 2) * state_size)
        for idx in tmp_sbox_indices:
            sboxes.append(var_x[1][idx])
            sboxes.append(var_x[3][idx])
    else:
        sboxes = [var_x[aes_state][i] for aes_state in range(config["aesstates"]) 
              for i in sbox_indices]

    model.addConstr(quicksum(sboxes) - activesboxes == 0, 
                    "Count Active S-boxes")

    return model
