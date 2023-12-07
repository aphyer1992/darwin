import math
import random
import time

random.seed('darwindesktopaa')


meat_nutrition = 1
bone_nutrition = 0.5
offal_nutrition = 0.25
global_food_details = {
    'names' : [ 'Meat', 'Bone', 'Offal', 'Grass', 'Fruit', 'Seeds' ],
    'nutritions' : [ meat_nutrition, bone_nutrition, offal_nutrition, 0.3, 0.7, 0.15 ],
    'costs' : [ 1, 4, 2, 5, 4, 1 ],
}

biome_food_densities = {
    'Tundra' : [ 0, 0, 0, 0.001, 0.00004, 0.0008 ],
    'Plains' : [ 0, 0, 0, 0.002, 0.0002, 0.001 ],
    'Jungle' : [ 0, 0, 0, 0.003, 0.001, 0.0012 ],
    }

biome_airdrop_densities = { #probably change these?
    'Tundra' : [ 0.01, 0.10, 0.17, 0.24, 0.05, 0.13],
    'Plains' : [ 0.01, 0.10, 0.17, 0.24, 0.05, 0.13],
    'Jungle' : [ 0.01, 0.10, 0.17, 0.24, 0.05, 0.13],
    }

max_speed = 10
starting_reserves_mult = 0.5
breeding_mult = 2
metabolic_rate = 0.01


global_num_foods = len(global_food_details['names'])
animals_to_make = []

speeds_in_round = [[],[],[],[],[],[],[],[],[],[]]
for speed in range(1,max_speed + 1):
    for round_num in range(max_speed):
        if math.floor(speed * round_num / max_speed) > math.floor(speed * (round_num- 1) / max_speed):
            speeds_in_round[round_num].append(speed)

def triangle(x):
    return(x*(x+1)*0.5)
def calc_size(speed, weapons, armor, edibles, camoflague):
    size = (0.5*triangle(speed)) + triangle(weapons) + (0.5 * triangle(armor)) + sum([global_food_details['costs'][x] for x in edibles])
    if camoflague:
        size = size * 2
    return(size)

def gen_animal():
    speed = random.choice(range(max_speed+1))
    speed = max(1, min(max_speed, speed))
    weapons = random.choice(range(-4,10))
    weapons = max(0, min(10, weapons))
    defense = random.choice(range(-1,10))
    defense = min(10,defense)
    armor = max(0, defense - (2*weapons))
    while armor > 2*(weapons + 1):
        armor = armor - 1
        weapons = weapons + 1
    camoflague = True if random.random() < 0.3 else False
    edibles = []
    for i in range(global_num_foods):
        if random.random() < 0.4:
            edibles.append(i)

    # check for obvious invalidity
    valid = True

    size = calc_size(speed, weapons, armor, edibles, camoflague)

    edibles = [e for e in edibles if global_food_details['nutritions'][e] > (size * metabolic_rate) ]

    if len(edibles) == 0:
        valid = False

    stat_string = 'W{}A{}S{}'.format(weapons, armor, speed)
    detail_strings = [stat_string]
    if camoflague:
        detail_strings.append('Camo')
    detail_strings.append('Eats {}'.format(' and '.join([global_food_details['names'][i] for i in edibles])))
    name = ', '.join(detail_strings)

    if valid == True:
        animals_to_make.append({
            'speed' : speed,
            'weapons' : weapons,
            'armor' : armor,
            'edibles' : edibles,
            'camoflague' : camoflague,
            'name' : name,
            'size': calc_size(speed, weapons, armor, edibles, camoflague)
        })

while len(animals_to_make) < 200:
    gen_animal()

def create_basic_herbivores(edibles, name_str): # make a locust, a speed invincible, an armor invincible, etc.
    animals_to_make.append({'speed':1, 'weapons':0, 'armor':0, 'edibles':edibles, 'camoflague':False, 'name':'{} Locust'.format(name_str), 'size': calc_size(1,0,0,edibles,False)})
    animals_to_make.append({'speed':7, 'weapons':0, 'armor':0, 'edibles':edibles, 'camoflague':False, 'name':'{} Speeder'.format(name_str), 'size': calc_size(7,0,0,edibles,False)})
    animals_to_make.append({'speed':4, 'weapons':0, 'armor':0, 'edibles':edibles, 'camoflague':False, 'name':'{} Half-Speeder'.format(name_str), 'size': calc_size(4,0,0,edibles,False)})
    animals_to_make.append({'speed':1, 'weapons':2, 'armor':5, 'edibles':edibles, 'camoflague':False, 'name':'{} Armor'.format(name_str), 'size': calc_size(1,2,5,edibles,False)})
    animals_to_make.append({'speed':1, 'weapons':1, 'armor':3, 'edibles':edibles, 'camoflague':False, 'name':'{} Half-Armor'.format(name_str), 'size': calc_size(1,1,3,edibles,False)})
    animals_to_make.append({'speed':3, 'weapons':0, 'armor':0, 'edibles':edibles, 'camoflague':True, 'name':'{} Fast Camo Locust'.format(name_str), 'size': calc_size(3,0,0,edibles,True)})
    animals_to_make.append({'speed':1, 'weapons':0, 'armor':0, 'edibles':edibles, 'camoflague':True, 'name':'{} Slow Camo Locust'.format(name_str), 'size': calc_size(1,0,0,edibles,True)})
    if 0 not in edibles:
        edibles = [0]+edibles
    animals_to_make.append({'speed':1, 'weapons':1, 'armor':0, 'edibles':edibles, 'camoflague':True, 'name':'{} Flytrap'.format(name_str), 'size': calc_size(1,1,0,edibles,True)})
    animals_to_make.append({'speed':3, 'weapons':1, 'armor':0, 'edibles':edibles, 'camoflague':True, 'name':'{} Spider'.format(name_str), 'size': calc_size(3,1,0,edibles,True)})

for i in range(global_num_foods):
    create_basic_herbivores([i], global_food_details['names'][i])

for i in range(2,11):
    animals_to_make.append({'speed':1, 'weapons':i, 'armor':0, 'edibles':[0], 'camoflague':True, 'name':'Lurker {}'.format(i), 'size': calc_size(1,i,0,[0],True)})

for i in range(1,8):
    animals_to_make.append({'speed':i, 'weapons':i, 'armor':0, 'edibles':[0], 'camoflague':False, 'name':'Hunter {}'.format(i), 'size': calc_size(i,i,0,[0],False)})
    animals_to_make.append({'speed':i, 'weapons':i, 'armor':0, 'edibles':[0,2], 'camoflague':False, 'name':'Offal Hunter {}'.format(i), 'size': calc_size(i,i,0,[0,2],False)})
    animals_to_make.append({'speed':i, 'weapons':i, 'armor':0, 'edibles':[0,1], 'camoflague':False, 'name':'Bone Hunter {}'.format(i), 'size': calc_size(i,i,0,[0,1],False)})
    animals_to_make.append({'speed':i, 'weapons':i, 'armor':0, 'edibles':[0,4], 'camoflague':False, 'name':'Fruit Hunter {}'.format(i), 'size': calc_size(i,i,0,[0,4],False)})
    animals_to_make.append({'speed':i, 'weapons':i, 'armor':0, 'edibles':[0,3], 'camoflague':False, 'name':'Grass Hunter {}'.format(i), 'size': calc_size(i,i,0,[0,3],False)})


create_basic_herbivores([3,4,5], 'Multiplant')

animals_to_make.sort(key=lambda x:x['speed'])
animals_to_make.sort(key=lambda x:x['armor'])
animals_to_make.sort(key=lambda x:x['weapons'])
animals_to_make.sort(key=lambda x:x['size'])

global_animal_names = []
global_animal_speeds = []
global_animal_sizes = []
global_animal_edibles = []
global_animal_weapons = []
global_animal_armor = []
global_animal_camoflague = []
global_animal_sizes = []

for a in animals_to_make:
    global_animal_speeds.append(a['speed'])
    global_animal_weapons.append(a['weapons'])
    global_animal_armor.append(a['armor'])
    global_animal_edibles.append(a['edibles'])
    global_animal_camoflague.append(a['camoflague'])
    global_animal_names.append(a['name'])
    global_animal_sizes.append(calc_size(a['speed'], a['weapons'], a['armor'], a['edibles'], a['camoflague']))

global_num_animals = len(global_animal_names)

global_food_chain = []
for a1 in range(global_num_animals):
    food_map = []
    for a2 in range(global_num_animals):
        can_kill = False
        if (global_animal_weapons[a1] > ((global_animal_weapons[a2])+ global_animal_armor[a2])):
            can_kill = True
        food_map.append(can_kill)
    global_food_chain.append(food_map)

global_chase_map = []
for a1 in range(global_num_animals):
    chase_map = []
    for a2 in range(global_num_animals):
        can_kill = global_food_chain[a1][a2]
        if can_kill and global_animal_speeds[a1] > global_animal_speeds[a2]:
            chase_map.append(True)
        else:
            chase_map.append(False)
    global_chase_map.append(chase_map)

def roll_die(n):
    return(math.ceil(random.random()*n))

def get_vectors_of_size_2():
    return([[0,2], [2,0], [0,-2], [-2,0], [1,1], [1,-1], [-1,1], [-1,-1]])

def get_vectors_of_size_1():
    return([[0,1], [1,0], [0,-1], [-1,0]])

def rand_expect(n):
    val = math.floor(n)
    if random.random() < (n - val):
        val = val + 1
    return(val)

def add_coords(coords, move):
    return([coords[0] + move[0], coords[1] + move[1]])

def weighted_random(input_list):
    rand = random.random()
    for e in input_list:
        rand = rand - e['prob']
        if rand < 0:
            return(e)
    assert(False)

class Map:
    def __init__(self, biome_map, biome_width, biome_height, gate_width):
        self.biome_width = biome_width
        self.biome_height = biome_height
        self.biome_map = biome_map
        self.biome_map_height = len(self.biome_map)
        self.biome_map_width = len(self.biome_map[0])
        self.height = self.biome_height * self.biome_map_height
        self.width = self.biome_width * self.biome_map_width
        self.gate_width = gate_width
        self.gate_row_min = math.floor(self.biome_height / 2) - math.floor(self.gate_width / 2)
        self.gate_row_max = math.floor(self.biome_height / 2) + math.ceil(self.gate_width / 2) - 1
        self.gate_col_min = math.floor(self.biome_width / 2) - math.floor(self.gate_width / 2)
        self.gate_col_max = math.floor(self.biome_width / 2) + math.ceil(self.gate_width / 2) - 1

        self.foods_map = []
        self.animals_map = []
        self.animals = []
        for row_num in range(self.height):
            food_row = []
            animal_row = []
            for col_num in range(self.width):
                cell_foods = []
                for f in global_food_details['names']:
                    cell_foods.append(0)

                food_row.append(cell_foods)
                animal_row.append([])
            self.foods_map.append(food_row)
            self.animals_map.append(animal_row)
        self.round_no = 0
        self.setup_logs()
        self.latest_update_time = time.localtime()
        self.deaths = []
        self.setup_biome_probs()
        self.exec_airdrop()

    def add_animal(self, species_id, row=None, col=None):
        if row is None:
            assert( col is None )
            biome = weighted_random(self.biome_probs)
            biome_offset = self.get_biome_offset([biome['row'], biome['col']])
            row = roll_die(self.biome_height) - 1 + biome_offset[0]
            col = roll_die(self.biome_width) - 1 + biome_offset[1]
        self.animals_map[row][col].append(species_id)
        self.animals.append({
            'species_id' : species_id,
            'row': row,
            'col': col,
            'size': global_animal_sizes[species_id],
            'speed': global_animal_speeds[species_id],
            'camoflague' : global_animal_camoflague[species_id],
            'weapons' : global_animal_weapons[species_id],
            'reserves' : global_animal_sizes[species_id]*starting_reserves_mult,
            'has_been_killed' : False, # for consistency we do not remove it from the array until the phase is over, but want to make sure it is not eaten twice and does not act later in that round.
        })

    def biome_from_square(self, square):
        return([math.floor(square[0]/self.biome_height), math.floor(square[1]/self.biome_width)])

    def dist_between_squares(self, s1, s2):
        return(abs(s1[0]-s2[0]) + abs(s1[1]-s2[1]))

    def squared_dist_between_squares(self, s1, s2):
        return((s1[0]-s2[0])**2 + (s1[1]-s2[1])**2)
    
    def shortest_dist_to_threat(self, square, threats):
        dists = [self.squared_dist_between_squares(square, t) for t in threats] #TODO: decide on this one!
        return(min(dists))

    def plant_eaten_in_square(self,species_id, square):
        foods_present = self.foods_map[square[0]][square[1]]
        foods_edible = global_animal_edibles[species_id]
        foods_both = [f for f in foods_edible if foods_present[f] > 0]
        nutritions_available = [global_food_details['nutritions'][f] for f in foods_both]
        if( len(nutritions_available) ):
            nutrition_gained = max(nutritions_available)
            food_eaten = random.choice([f for f in foods_both if global_food_details['nutritions'][f] == nutrition_gained])
        else:
            nutrition_gained = 0
            food_eaten = None
        return([nutrition_gained, food_eaten])

    def prey_eaten_in_square(self, species_id, square):
        animals_present = self.animals_map[square[0]][square[1]]
        prey_available = [a for a in animals_present if global_food_chain[species_id][a] == True]
        nutritions_available = [global_animal_sizes[a] for a in prey_available]
        if( len(nutritions_available) ):
            nutrition_gained = max(nutritions_available)
            prey_eaten = random.choice([p for p in prey_available if global_animal_sizes[p] == nutrition_gained])
        else:
            nutrition_gained = 0
            prey_eaten = None
        return([nutrition_gained, prey_eaten])

    def food_eaten_in_square(self, species_id, square):
        [plant_nutrition, plant_eaten] = self.plant_eaten_in_square(species_id, square)
        [prey_nutrition, prey_eaten] = self.prey_eaten_in_square(species_id, square)
        if prey_nutrition > plant_nutrition:
            return([prey_nutrition, prey_eaten, None])
        elif plant_nutrition > 0:
            return([plant_nutrition, None, plant_eaten])
        else:
            return([0, None, None])

    def total_food_in_square(self, species_id, square):
        [plant_nutrition, plant_eaten] = self.plant_eaten_in_square(species_id, square)
        [prey_nutrition, prey_eaten] = self.prey_eaten_in_square(species_id, square)
        if prey_nutrition > plant_nutrition:
            return([prey_nutrition, prey_eaten, None])
        elif plant_nutrition > 0:
            return([plant_nutrition, None, plant_eaten])
        else:
            return([0, None, None])

    def get_biome_offset(self, biome_coords):
        return([biome_coords[0]*self.biome_height, biome_coords[1]*self.biome_width])

    def airdrop_food_into_biome(self, food_id, biome_coords, drop_rate):
        food_to_make = rand_expect(drop_rate * self.biome_height * self.biome_width)
        biome_offset = self.get_biome_offset(biome_coords)
        for i in range(food_to_make):
            row_num = roll_die(self.biome_height) - 1 + biome_offset[0]
            col_num = roll_die(self.biome_width) - 1 + biome_offset[1]
            self.foods_map[row_num][col_num][food_id] = min(self.foods_map[row_num][col_num][food_id] + 1, 10)

    def exec_grow_in_biome(self, biome_coords):
        biome_type = self.biome_map[biome_coords[0]][biome_coords[1]]
        food_abundances = biome_food_densities[biome_type]
        for food_id in range(global_num_foods):
            food_abundance = food_abundances[food_id]
            self.airdrop_food_into_biome(food_id, biome_coords, food_abundance)
            
    def starting_airdrop_in_biome(self, biome_coords):
        biome_type = self.biome_map[biome_coords[0]][biome_coords[1]]
        food_abundances = biome_airdrop_densities[biome_type]
        for food_id in range(global_num_foods):
            food_abundance = food_abundances[food_id]
            self.airdrop_food_into_biome(food_id, biome_coords, food_abundance)
            
    def exec_grow(self):
        for row in range(self.biome_map_height):
            for col in range(self.biome_map_width):
                self.exec_grow_in_biome([row, col])
                
    def exec_airdrop(self):
        for row in range(self.biome_map_height):
            for col in range(self.biome_map_width):
                self.starting_airdrop_in_biome([row, col])
                
    def move_animal(self, animal, move):
        #print('{} moves from {},{} to {},{}'.format(animal['species_id'], animal['row'], animal['col'], move[0], move[1]))
        old_biome = self.biome_from_square([animal['row'],animal['col']])
        new_biome = self.biome_from_square(move)
        if old_biome != new_biome:
            print('{} moves from {},{} to {},{}'.format(animal['species_id'], animal['row'], animal['col'], move[0], move[1]))
        self.animals_map[animal['row']][animal['col']].remove(animal['species_id'])
        animal['row'] = move[0]
        animal['col'] = move[1]
        self.animals_map[animal['row']][animal['col']].append(animal['species_id'])

    def looks_dangerous_in_square(self, species_id, square, can_see_camo):
        animals_present = self.animals_map[square[0]][square[1]]
        predators_present = [a for a in animals_present if global_food_chain[a][species_id] == True]
        if can_see_camo==False:
            predators_present = [p for p in predators_present if global_animal_camoflague[p] == False]

        if len(predators_present):
            return(True)
        else:
            return(False)

    def best_food_in_square(self, species_id, square):
        best_food = 0
        foods_edible = global_animal_edibles[species_id]
        for f in foods_edible:
            if self.foods_map[square[0]][square[1]][f] > 0:
                best_food = max(best_food, global_food_details['nutritions'][f])
        return(best_food)
    
    def food_amount_in_square(self, species_id, square):
        food_amount = 0
        foods_edible = global_animal_edibles[species_id]
        for f in foods_edible:
            food_amount = food_amount + (self.foods_map[square[0]][square[1]][f] * global_food_details['nutritions'][f])
        return(food_amount)

    def prey_list_in_square(self, species_id, square, can_see_camo, must_chase=False):
        animals_present = self.animals_map[square[0]][square[1]]
        if must_chase:  # if we are looking 2 away, we must be able to chase them as well
            prey_present = [a for a in animals_present if global_chase_map[species_id][a] == True]
        else:           # if they are adjacent, we just must be able to eat them
            prey_present = [a for a in animals_present if global_food_chain[species_id][a] == True]
        if can_see_camo==False:
            prey_present = [p for p in prey_present if global_animal_camoflague[p] == False]
        return(prey_present)

    def prey_amount_in_square(self, species_id, square, can_see_camo, must_chase=False):
        prey_present = self.prey_list_in_square(species_id, square, can_see_camo, must_chase)
        return(sum([global_animal_sizes[a] for a in prey_present]))

    def kill_animal(self, animal, killed_by):
        self.animals_map[animal['row']][animal['col']].remove(animal['species_id'])
        animal['has_been_killed'] = True
        output_meat = ( 0.4 * animal['size'] ) + ( 0.667 * animal['reserves'] )
        output_bone = ( 0.4 * animal['size'] )
        output_offal = ( 0.2 * animal['size'] ) + ( 0.333 * animal['reserves'] )
        self.foods_map[animal['row']][animal['col']][0] = self.foods_map[animal['row']][animal['col']][0] + rand_expect( output_meat / meat_nutrition )
        self.foods_map[animal['row']][animal['col']][1] = self.foods_map[animal['row']][animal['col']][1] + rand_expect( output_bone / bone_nutrition )
        self.foods_map[animal['row']][animal['col']][2] = self.foods_map[animal['row']][animal['col']][2] + rand_expect( output_offal / offal_nutrition )
        self.deaths.append({'round' : self.round_no, 'victim' : animal['species_id'], 'killed_by' : killed_by})

    def valid_square(self, square):
        if square[0] >= 0 and square[0] < self.height and square[1] >= 0 and square[1] < self.width:
            return True
        else:
            return False

    def in_gate_row(self, square):
        r = square[0] % self.biome_height
        if r >= self.gate_row_min and r <= self.gate_row_max:
            return True
        else:
            return False

    def in_gate_col(self, square):
        c = square[1] % self.biome_width
        if c >= self.gate_col_min and c <= self.gate_col_max:
            return True
        else:
            return False

    def get_biome_coords(self, square):
        return([math.floor(square[0]/self.biome_height), math.floor(square[1]/self.biome_width)])
    
    def squares_can_see(self, s1, s2):
        b1 = self.get_biome_coords(s1)
        b2 = self.get_biome_coords(s2)
        if b1[0] != b2[0] and self.in_gate_col(s1) == False and self.in_gate_col(s2) == False:
            return False
                
        if b1[1] != b2[1] and self.in_gate_row(s1) == False and self.in_gate_row(s2) == False:
            return False
                
        return True
    
    def exec_actions(self):
        random.shuffle(self.animals)
        self.animals.sort(key = lambda x: x['weapons']) # avoid catching same-speed via turn order shenanigans
        self.animals.sort(key = lambda x: x['size']) # avoid catching same-speed via turn order shenanigans
        self.animals.sort(key = lambda x: x['speed'], reverse=True)
        speeds_to_act = speeds_in_round[self.round_no%max_speed]

        for a in self.animals:
            if a['has_been_killed'] == True:
                continue

           # if( random.random() < (a['speed'] / max_speed)): # if we act
            if a['speed'] in speeds_to_act:
                current_square = [a['row'], a['col']]
                neighbor_squares = [add_coords(current_square, x) for x in get_vectors_of_size_1()]
                neighbor_squares = [s for s in neighbor_squares if self.valid_square(s)]
                neighbor_squares = [s for s in neighbor_squares if self.squares_can_see(current_square, s)]
                two_away_squares = [add_coords(current_square, x) for x in get_vectors_of_size_2()]
                two_away_squares = [s for s in two_away_squares if self.valid_square(s)]
                two_away_squares = [s for s in two_away_squares if self.squares_can_see(current_square, s)]

                random.shuffle(neighbor_squares) # order will be used for deciding where to move to later
                random.shuffle(two_away_squares)

                possible_moves = [current_square] + neighbor_squares # note that current square is in front
                visible_squares = possible_moves + two_away_squares

                # threats get calced separately because of camo
                adj_threats = [s for s in possible_moves if self.looks_dangerous_in_square(a['species_id'], s, can_see_camo=True)]
                two_away_threats = [s for s in two_away_squares if self.looks_dangerous_in_square(a['species_id'], s, can_see_camo=False)]
                threats = adj_threats + two_away_threats

                if len(threats):
                    possible_moves = [[x, self.shortest_dist_to_threat(x, threats)] for x in possible_moves]
                    best_dist = max([x[1] for x in possible_moves]) # furthest we can get from a threat
                    best_dist = min(best_dist, 4.01) # out of vision range is always okay
                    possible_moves = [x[0] for x in possible_moves if x[1] >= best_dist]

                # adjacent to us we prioritize prey over food
                possible_moves = [[square, self.prey_amount_in_square(a['species_id'], square, can_see_camo=True)] for square in possible_moves]
                best = max([x[1] for x in possible_moves])
                if best == 0:
                    possible_moves = [[x[0], self.best_food_in_square(a['species_id'], x[0])] for x in possible_moves]
                    best = max([x[1] for x in possible_moves])

                if best == 0: # we see no food or prey we can get now
                    #distant_targets = [[square, self.food_amount_in_square(a['species_id'], square)] for square in two_away_squares]
                    #best = max([x[1] for x in distant_targets])
                    # if no food 2 away we check for prey, but prey 2 away is prioritized below food because it can get away and camo animals won't chase
                    #if (best == 0): # and (a['camoflague'] == False) ?
                    distant_targets = [[square, self.prey_amount_in_square(a['species_id'], square, can_see_camo=False, must_chase=True)] for square in two_away_squares]
                    best = max([x[1] for x in distant_targets])

                    if best > 0:
                        best_target_hits = [x[0] for x in distant_targets if x[1] == best]
                        best_target = best_target_hits[0]
                        possible_moves = [[x[0], self.dist_between_squares(x[0], best_target)] for x in possible_moves]
                        best_dist = min([x[1] for x in possible_moves])
                        possible_moves = [x[0] for x in possible_moves if x[1] == best_dist]
                    else:
                        possible_moves = [x[0] for x in possible_moves]
                        possible_moves.reverse() # current square should go last so we will always move if we are in the middle of nowhere with nothing to eat
                        
                    move_to = possible_moves[0]
                else:
                    possible_moves = [x[0] for x in possible_moves if x[1] == best]
                    move_to = possible_moves[0] # move to first one to favor stillness if there is food in our current square.

                if move_to != current_square:
                    self.move_animal(a, move_to)

                # if there is prey in our new square, we kill the biggest
                prey_list = self.prey_list_in_square(a['species_id'], move_to, can_see_camo=True)
                if len(prey_list):
                    biggest_size = max([global_animal_sizes[p] for p in prey_list])
                    biggest_prey = [p for p in prey_list if global_animal_sizes[p] == biggest_size]
                    prey_species = random.choice(biggest_prey)
                    target_animals = [a for a in self.animals if a['row'] == move_to[0] and a['col'] == move_to[1] and a['species_id'] == prey_species and  a['has_been_killed'] == False]
                    target_animal = random.choice(target_animals)
                    self.kill_animal(target_animal, a['species_id'])

                # if there is food in our new square, we eat one of the best
                foods_edible = global_animal_edibles[a['species_id']]
                foods_present = [f for f in foods_edible if self.foods_map[move_to[0]][move_to[1]][f] > 0]
                if len(foods_present):
                    best_nutrition = max([global_food_details['nutritions'][f] for f in foods_present])
                    best_food = random.choice([f for f in foods_present if global_food_details['nutritions'][f] == best_nutrition])
                    a['reserves'] = a['reserves'] + best_nutrition
                    self.foods_map[move_to[0]][move_to[1]][best_food] = self.foods_map[move_to[0]][move_to[1]][best_food] - 1

                # whether we moved or not, there is a cost for being active
                a['reserves'] = a['reserves'] - (a['size'] * metabolic_rate)


        # eaten animals are removed here so that self.animals will not screw up as we iterate through it
        self.animals = [a for a in self.animals if a['has_been_killed'] == False]

    def exec_breeding_hunger(self):
        for a in self.animals:
            if a['reserves'] < 0:
                self.kill_animal(a, None)
            elif a['reserves'] > (breeding_mult * a['size']):
                a['reserves'] = a['reserves'] - (a['size'] * (1 + starting_reserves_mult))
                self.add_animal(a['species_id'], a['row'], a['col'])

        self.animals = [a for a in self.animals if a['has_been_killed'] == False]

    def exec_round(self):
        self.round_no = self.round_no + 1
        self.exec_grow()
        self.exec_actions()
        self.exec_breeding_hunger()
        if self.round_no % 10 == 0:
            self.log_counts()

    def log_row(self, row, mode='a'):
        row = [str(e) for e in row]
        row = [s.replace(',',';') for s in row]
        log_string = ','.join(row)+'\n'
        f = open('darwin_map_output.csv', mode)
        f.write(log_string)
        
    def get_animal_counts(self):
        num_biomes = self.biome_map_width * self.biome_map_height + 1
        counts = [ [0 for b in range(num_biomes)] for n in global_animal_names]
        for a in self.animals:
            counts[a['species_id']][0] = counts[a['species_id']][0] + 1
            biome = self.get_biome_coords([a['row'], a['col']])
            biome_index = 1 + biome[1] + (biome[0] * self.biome_map_width)
            counts[a['species_id']][biome_index] = counts[a['species_id']][biome_index] + 1
        return(counts)

    def log_counts(self):
        log_row = [self.round_no] + self.get_animal_counts()
        self.log_row(log_row)

    def setup_logs(self):
        log_row = ['Generation']
        for n in global_animal_names:
            log_row.append(n)
        self.log_row(log_row, mode='w')

    def show(self):
        print('Round {}\n'.format(self.round_no))
        print('Food:\n')
        for row in self.foods_map:
            print(row)
        print('\nAnimals:\n')
        for row in self.animals_map:
            print(row)

    def render_square(self, square):
        foods = self.foods_map[square[0]][square[1]]
        content_strings = []
        for i in range(global_num_foods):
            if foods[i] > 0:
                content_strings.append('{}{}'.format(global_food_details['names'][i][0], foods[i]))

        animals = self.animals_map[square[0]][square[1]]
        for a in animals:
            content_strings.append('({})'.format(a))
        return(' '.join(content_strings))

    def show_area(self, corner_square, size):
        out = []
        max_len = 0
        for i in range(size):
            row = []
            rownum = corner_square[0] + i
            for j in range(size):
                colnum = corner_square[1]+j
                if rownum >= self.height or rownum < 0 or colnum >= self.width or colnum < 0:
                    contents = 'X'
                else:
                    contents = self.render_square(self.add_coords(corner_square,[i,j]))
                row.append(contents)
                max_len = max(max_len, len(contents))
            out.append(row)
        for row in out:
            for entry in row:
                while len(entry) < max_len:
                    entry = entry + ' '
        for row in out:
            print([e + (' ' * (max_len - len(e))) for e in row])

    def setup_biome_probs(self):
        total_nutrition = 0
        prob_list = []
        for r in range(self.biome_map_height):
            for c in range(self.biome_map_width):
                n = self.total_nutrition_per_round_in_biome([r,c])
                total_nutrition = total_nutrition + n
                prob_list.append({'row':r, 'col': c, 'nutrition': n})
        for e in prob_list:
            e['prob'] = e['nutrition'] / total_nutrition

        self.biome_probs = prob_list
                
    def total_nutrition_per_round_in_biome(self, biome_coords):
        biome_type = self.biome_map[biome_coords[0]][biome_coords[1]]
        square_nutrition = 0
        for food_id in range(global_num_foods):
            food_abundance = biome_food_densities[biome_type][food_id]
            per_food_nutrition = global_food_details['nutritions'][food_id]
            food_nutrition = per_food_nutrition * food_abundance
            square_nutrition = square_nutrition + food_nutrition
        total_nutrition = square_nutrition * self.biome_width * self.biome_height
        return(total_nutrition)
        
    def total_nutrition_per_round(self):
        nutrition = 0
        for r in range(self.biome_map_height):
            for c in range(self.biome_map_width):
                nutrition = nutrition + self.total_nutrition_per_round_in_biome([r,c])
        return(nutrition)

    def total_nutrition_rate(self):
        counts = self.get_animal_counts()
        total_rate = 0
        for i in range(global_num_animals):
            total_rate = total_rate + (counts[i][0] * global_animal_sizes[i] * global_animal_speeds[i] * metabolic_rate / max_speed)
        return(total_rate)

    def total_food_amount(self, food_id):
        return(sum([sum([square[food_id] for square in row]) for row in self.foods_map]))

    def display_status(self):
        new_time = time.localtime()
        elapsed = time.mktime(new_time) - time.mktime(self.latest_update_time)
        self.latest_update_time = new_time
        counts = self.get_animal_counts()
        print('\nRound {}. {}s elapsed since last update. ({} species remain of which {} have real populations, {:.2f}% of capacity):\n\n'.format(
            my_map.round_no,
            elapsed,
            len([i for i in counts if i[0] > 0]),
            len([i for i in counts if i[0] >= 10]),
            100 * my_map.total_nutrition_rate() / my_map.total_nutrition_per_round()
        ))
        for i in range(global_num_animals):
            if counts[i][0] >= 10:
                biome_breakdown = []
                for j in range(1, len(counts[i])):
                    biome_breakdown.append(str(counts[i][j]))
                biome_breakdown = ', '.join(biome_breakdown)
                print('{} ({}) has a population of {} ({})'.format(i, global_animal_names[i], counts[i][0], biome_breakdown))
        food_strings = ['{} density is {:.4f}'.format(global_food_details['names'][f], self.total_food_amount(f) / (self.height * self.width)) for f in range(global_num_foods)]
        print(', '.join(food_strings))

        starved = [d for d in self.deaths if d['killed_by'] is None]
        print('{} animals have starved'.format(len(starved)))
        killed = [d for d in self.deaths if d['killed_by'] is not None]
        predation_map = {}
        for k in killed:
            k_string = '{} killed {}'.format(k['killed_by'], k['victim'])
            if k_string in predation_map.keys():
                    predation_map[k_string] = predation_map[k_string] + 1
            else:
                    predation_map[k_string] = 1
        predation_list = []
        for k in predation_map.keys():
            predation_list.append(['{} {} times'.format(k, predation_map[k]), predation_map[k]])
        predation_list.sort(key=lambda x:x[1], reverse=True)
        print('{} animals have been killed by {} predation relationships, e.g.'.format(len(killed), len(predation_list)))
        num_to_show = min(len(predation_list), 5)
        if num_to_show > 0:
            predation_strings = []
            for i in range(num_to_show):
                predation_strings.append(predation_list[i][0])
            print(', '.join(predation_strings))
        self.deaths = []
        
    def focus_animal(self, species_id):
        f = [a for a in self.animals if a['species_id'] == species_id][0]
        for a in self.animals:
            if a['species_id'] == species_id and a['row'] == f['row'] and a['col'] == f['col']:
                a['focus'] = True
 
    def show_focus(self):
        f = [a for a in self.animals if 'focus' in a.keys()]
        assert(len(f)==1)
        self.show_area([f[0]['row']-3, f[0]['col']-3], 7)

full_map = True
if full_map:
    my_map = Map([['Plains', 'Tundra', 'Plains','Jungle', 'Plains']], 300, 300, 0)  # NOTE: UNLESS I SCREWED THINGS UP, THIS LAST 0 MEANS NO BIOME TRANSIT.

    nutrition_per_species = my_map.total_nutrition_per_round() / global_num_animals
    for i in range(global_num_animals):
        num_to_make = math.floor(nutrition_per_species / (global_animal_sizes[i] * (global_animal_speeds[i]/max_speed) * metabolic_rate))
        for j in range(num_to_make):
            my_map.add_animal(i)

    my_map.display_status()
    while True:
        my_map.exec_round()
        if my_map.round_no < 10000:
            for i in range(global_num_animals):
                chance_to_make = nutrition_per_species / (20000*global_animal_sizes[i] * (global_animal_speeds[i]/max_speed) * metabolic_rate)
                if random.random() < chance_to_make:
                    my_map.add_animal(i)
        elif my_map.round_no < 100000:
            for i in range(global_num_animals):
                chance_to_make = nutrition_per_species / (200000*global_animal_sizes[i] * (global_animal_speeds[i]/max_speed) * metabolic_rate)
                if random.random() < chance_to_make:
                    my_map.add_animal(i)

        if my_map.round_no%100 == 0:
            my_map.display_status()
            if my_map.round_no > 1e6:
                break
else:
    my_map = Map(5,5,[[0,0.005]])
    my_map.add_animal(9)
    my_map.add_animal(15)
    my_map.add_animal(15)
    #for i in range(50):
    #    my_map.show()
    #    my_map.exec_round()
