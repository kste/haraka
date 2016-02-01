"""
Constraints for AES-like round functions.
"""

from gurobipy import *

def addAESrndconstraints(gurobi_model, state_dim, var_x, var_d, branch_number,
                         rounds):
    """
    Adds constraints from MixColumns/ShiftRows for given branch number.
    """
    state = [[x*state_dim + y for x in range(state_dim)] 
             for y in range(state_dim)]

    next_index = state_dim * state_dim
    dummy = 0

    var_mcintmp = []
    var_mcouttmp = []
    for col in range(rounds * state_dim):
        var_mcintmp.append(gurobi_model.addVar(vtype=GRB.BINARY, 
                                               name="isMCactiveIn[{}]".format(col)))
        var_mcouttmp.append(gurobi_model.addVar(vtype=GRB.BINARY, 
                                                name="isMCactiveOut[{}]".format(col)))
    gurobi_model.update()

    for rnd in range(rounds):
        #Shiftrows
        tmp = [0 for x in range(state_dim)]
        for i in range(1, state_dim):
            for j in range(state_dim):
                tmp[j] = state[i][(j + i) % state_dim]
            for j in range(state_dim):
                state[i][j] = tmp[j]
        #MixColumns
        for j in range(state_dim):
            tmp_before = []
            tmp_after = []
            for i in range(state_dim):
                tmp_before.append(state[i][j])
            for i in range(state_dim - 1):
                tmp_after.append(next_index + i)
            tmp_after.append(next_index + (state_dim - 1))
            #Limit for branch number
            gurobi_model.addConstr(quicksum(var_x[i] for i in tmp_before +
                                            tmp_after) - (branch_number) *
                                   var_d[dummy] >= 0, "MC{}{}".format(rnd, j))

            #Force both sides to be either zero or non-zero
            gurobi_model.addConstr(quicksum(var_x[i] for i in tmp_before) >= 
                                   var_mcintmp[rnd*state_dim + j], "MCactivein")
            gurobi_model.addConstr(quicksum(var_x[i] for i in tmp_after) >= 
                                   var_mcouttmp[rnd*state_dim + j], "MCactiveout")
            gurobi_model.addConstr(quicksum(var_x[i] for i in tmp_before + tmp_after) <=
                                   var_mcintmp[rnd*state_dim + j] * 
                                   var_mcouttmp[rnd*state_dim + j] * 2 *
                                   state_dim, "MCValid{}{}".format(rnd, j))

            for i in range(state_dim):
                gurobi_model.addConstr(var_d[dummy] - var_x[state[i][j]] >= 0,
                                       "MCt{}{}{}".format(rnd, j, i))
            for i in range(state_dim):
                state[i][j] = next_index
                next_index += 1
                gurobi_model.addConstr(var_d[dummy] - var_x[state[i][j]] >= 0,
                                       "MCt{}{}{}".format(rnd, j, i))
            dummy += 1
    return gurobi_model
