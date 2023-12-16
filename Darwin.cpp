// Darwin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <array>
#include <random>
#include <set>
#include <memory>
#include <chrono>
#include <ctime>

using namespace std;

namespace constants
{
    const float meat_nutrition = 1;
    const float bone_nutrition = 0.5;
    const float offal_nutrition = 0.25;
    const float fruit_nutrition = 0.7;
    const float grass_nutrition = 0.3;
    const float seeds_nutrition = 0.15;

    const int num_foods = 6;
    // IMPORTANT: MUST BE SORTED DESCENDING BY NUTRITION!
    const string food_names[num_foods] = { "Meat", "Fruit", "Bone", "Grass", "Offal", "Seeds" };
    const array<float, num_foods> food_nutritions = { meat_nutrition, fruit_nutrition, bone_nutrition, grass_nutrition, offal_nutrition, seeds_nutrition };
    const array<float, num_foods> food_costs = {1, 3, 4, 5, 2, 1};
    const array<float, num_foods> food_spawn_rates = { 0, 0.0002, 0, 0.002, 0, 0.001 };
    const array<float, num_foods> food_drop_rates = { 0.5, 0, 0.25, 0, 0.25, 0 };
    const array<float, num_foods> food_starting_rates = { 0.01, 0.05, 0.10, 0.24, 0.17, 0.13 };

    const int max_speed = 10;
    const int max_weapons_armor = 10;

    const int num_species = 300;

    const float starting_reserves_ratio = 0.5;
    const float reproduction_reserves_ratio = 2.0;
    const float metabolic_rate = 0.01;

    const array<array<int, 2>, 5> dist_01_neighbors = { { {0,0}, {0,1}, {0,-1}, {1,0}, {-1,0} } }; // why do i need a third brace?
    const array<array<int, 2>, 8> dist_2_neighbors = { { {0,2}, {0,-2}, {2,0}, {-2,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1} } };
    
}

int triangle(int n)
{
    return(n * (n + 1) / 2);
}

float calc_size(int weapons, int armor, int speed, array <bool,6> edibles, bool camo)
{
    float size = 0;
    size = size + triangle(weapons);
    size = size + (0.5 * triangle(armor));
    size = size + (0.5 * triangle(speed));

    for (int i = 0; i < constants::num_foods; i++) {
        size = size + (edibles[i] ? constants::food_costs[i] : 0);
    }

    size = size * (camo ? 2 : 1);
    return(size);
}

class Coords {
public:
    int row;
    int col;

    Coords() {
        row = -1;
        col = -1;
    }

    Coords(int row_in, int col_in) {
        row = row_in;
        col = col_in;
    }

    Coords add_move(array<int, 2> move) {
        return(Coords(row + move[0], col + move[1]));
    }

    float dist_from(Coords target) {
        return(sqrt(pow(row - target.row, 2) + pow(col - target.col, 2)));
    }

    float closest_dist_to(vector<Coords> targets) {
        float dist = 999999; // I tried making this INT_MAX and it blew up and I don't understand why
        for (Coords target : targets) {
            dist = min(dist, dist_from(target));
        }
        return(dist);
    }
};

class Species {
public:
    int weapons;
    int armor;
    int speed;
    array<bool, 6> edibles;
    bool camo;
    float size;
    float runrate;
    string name;
    int id;

    // stupid default constructor?
    Species()
    {
        name = "UNDEFINED";
    }

    string default_name()
    {
        string default_name = "W" + to_string(weapons) + "A" + to_string(armor) + "S" + to_string(speed);
        if (camo)
        {
            default_name = default_name + " Camo";
        }
        default_name = default_name + " Eats";
        for (int i = 0; i < constants::num_foods; i++) {
            if (edibles[i]) {
                default_name = default_name + " " + constants::food_names[i];
            }
        }
        return(default_name);
    }

    Species( int weapons_in, int armor_in, int speed_in, array<bool,6> edibles_in, bool camo_in, string name_in ="")
    {
        // I need to do this it seems...constructors with names equivalent are not working;
        weapons = weapons_in;
        armor = armor_in;
        speed = speed_in;
        edibles = edibles_in;
        camo = camo_in;
        name = name_in;

        size = calc_size(weapons, armor, speed, edibles, camo);
        runrate = size * speed * constants::metabolic_rate / constants::max_speed;

        if (name == "")
        {
            name = default_name();
        }
     }

    string print_details_string() {
        return(name + " (" + to_string(id) + ") : " + default_name());
    }

    void print() {
        cout << print_details_string() << "\n";
    }
};

class Animal {
public:
    Species species;
    Coords coords;
    float reserves;
    bool alive;
    long id;

    Animal() {

    }

    Animal(Species species_in, Coords coords_in, long id_in) {
        species = species_in;
        reserves = species.size * constants::starting_reserves_ratio;
        coords = coords_in;
        id = id_in;
        alive = true;
    }

    void exec_hunger() {
        reserves = reserves - (constants::metabolic_rate * species.size);
    }

    void print() {
        cout << "\nAnimal " << id << " Info:\n";
        cout << "Species: " << species.print_details_string() << "\n";
        cout << "Reserves: " << reserves << "\n";
        cout << "Alive: " << alive << "\n";
        cout << "Located at: (" << coords.row << "," << coords.col << ")\n\n";
    }
};

class Square {
public:
    Coords coords;
    array<int, constants::num_foods> foods_present;
    vector<shared_ptr<Animal>> animals_present;
    
    Square(Coords coords_in) {
        coords = coords_in;
        foods_present.fill(0);
        animals_present = {};
    }

    void add_animal(shared_ptr<Animal> to_add) {
        animals_present.push_back(to_add);
    }

    void clear_dead_animals() {
        vector<shared_ptr<Animal>> surviving_animals = {};
        for (shared_ptr<Animal> a : animals_present) {
            if ((*a).alive) {
                surviving_animals.push_back(a);
            }
        }
        animals_present = surviving_animals;
    }

    void clear_animal_by_id(long id) {
        vector<shared_ptr<Animal>> remaining = {};
        for (shared_ptr<Animal> a : animals_present) {
            if ((*a).id != id) {
                remaining.push_back(a);
            }
        }
        animals_present = remaining;
    }
};

class SquareInfo {
public:
    bool visible_threat;
    float best_food;
    float largest_visible_prey;
    float largest_chaseable_prey;
    float distance_from_threat;
    bool eligible_move;
    Coords coords;

    SquareInfo() {

    }

    SquareInfo(Coords coords_in) {
        coords = coords_in;
    }
};

class World {
public:
    int width;
    int height;
    int size;
    int species_defined;
    long animals_created;
    int rounds_elapsed;
    Species species_list[constants::num_species];
    array<array<bool, constants::num_species>, constants::num_species> predation_map;
    array<array<bool, constants::num_species>, constants::num_species> pursuit_map;
    std::mt19937 rng;
    vector<vector<Square>> contents;
    vector<shared_ptr<Animal>> animals_list;
    chrono::system_clock::time_point latest_update;
    int num_recently_killed;
    int num_recently_starved;
    vector<array<int, 2>> recent_kills;


    World(int width_in, int height_in)
    {
        width = width_in;
        height = height_in;
        size = width * height;
        species_defined = 0;
        animals_created = 0;
        rounds_elapsed = 0;
        setup_fixed_species();
        setup_random_species();
        sort_species_by_size();
        create_predation_maps();
        create_contents();
        spawn_foods(constants::food_starting_rates);
        airdrop_animals();
        latest_update = chrono::system_clock::now();
        recent_kills = {};
    };

    int get_random_number(int min, int max) {
        std::uniform_int_distribution<> dis(min, max);
        return(dis(rng));
    }

    int randomize_to_int(float input) {
        int out = floor(input);
        float remainder = input - out;
        if (get_random_number(0, 100) < (remainder * 100)) { // not perfect I guess
            out = out + 1;
        }
        return(out);
    }

    void add_species(int weapons, int armor, int speed, array<bool, 6> edibles, bool camo, string name="")
    {
        Species new_species = Species(weapons, armor, speed, edibles, camo, name);
        species_list[species_defined] = new_species;
        species_defined++;
    }

    void setup_fixed_herbivores(array<bool, 6> edibles, string name_prefix)
    {
        add_species(0, 0, 1, edibles, false, name_prefix + " Locust");
        add_species(0, 0, 1, edibles, true, name_prefix + " Slow Camo Locust");
        add_species(0, 0, 3, edibles, true, name_prefix + " Fast Camo Locust");
        add_species(0, 0, 3, edibles, false, name_prefix + " Half-Speeder");
        add_species(0, 0, 5, edibles, false, name_prefix + " Speeder");
        add_species(1, 3, 1, edibles, false, name_prefix + " Half-Armor");
        add_species(2, 5, 1, edibles, false, name_prefix + " Armor");
        add_species(3, 7, 1, edibles, false, name_prefix + " Invincible");

        edibles[0] = true; // these should eat meat
        add_species(1, 0, 1, edibles, true, name_prefix + " Flytrap");
        add_species(1, 0, 3, edibles, true, name_prefix + " Spider");
    }

    void setup_fixed_species()
    {
        for (int i = 0; i < constants::num_foods; i++)
        {
            array <bool,6> temp_edibles;
            temp_edibles.fill(false);
            temp_edibles[i] = true;
            setup_fixed_herbivores(temp_edibles, constants::food_names[i]);
        }
        array<bool, 6> multiplant_edibles = { false,true,false,true,false,true };
        setup_fixed_herbivores(multiplant_edibles, "Multiplant");

        array <bool, 6> meat_only = { true, false, false, false, false, false };

        for (int i = 1; i < 10; i++)
        {
            add_species(i, 0, 1, meat_only, true, "Lurker " + to_string(i));
        }

        for (int i = 0; i < constants::num_foods; i++)
        {
            array<bool,6> temp_edibles = {true,false,false,false,false };
            temp_edibles[i] = true;
            for (int j = 2; j < 10; j++)
            {
                add_species(j, 0, j, temp_edibles, false, constants::food_names[i] + " Hunter " + to_string(j));
            }
        }
    }

    void setup_random_species()
    {
        while (species_defined < constants::num_species)
        {
            int weapons = get_random_number(-2, constants::max_weapons_armor);
            weapons = max(0, weapons);
            int armor = get_random_number(0, constants::max_weapons_armor);
            armor = armor - weapons;
            armor = max(0, armor);
            int speed = get_random_number(1, constants::max_speed);
            bool camo = get_random_number(1, 3) == 2 ? true : false;
            array<bool, 6> edibles = { false, false, false, false, false, false };
            for (int i = 0; i < 6; i++)
            {
                if (get_random_number(1, 3) == 2)
                {
                    edibles[i] = true;
                }
            }
            add_species(weapons, armor, speed, edibles, camo, "Random Animal");
        }
    }

    void sort_species_by_size()
    {
        struct by_size {
            bool operator()(Species const& a, Species const& b) const {
                return a.size < b.size;
            }
        };

        std::sort(begin(species_list), end(species_list), [](Species const& a, Species const& b) {
            return a.size < b.size;
        });

        for (int i = 0; i < constants::num_species; i++) {
            species_list[i].id = i;
        }
    }

    void create_predation_maps()
    {
        cout << "Creating Predation Maps\n";

        for (int a = 0; a < constants::num_species; a++)
        {
            for (int b = 0; b < constants::num_species; b++)
            {
                Species s_a = species_list[a];
                Species s_b = species_list[b];
                if (s_a.weapons > (s_b.weapons + s_b.armor)) {
                    predation_map[a][b] = true;
                }
                else {
                    predation_map[a][b] = false;
                }
                pursuit_map[a][b] = ((s_a.weapons > (s_b.weapons + s_b.armor)) and (s_a.speed > s_b.speed)) ? true : false;
            }
        }
    }

    bool is_on_map(Coords coords) {
        if (coords.row < 0 || coords.row >= height || coords.col < 0 || coords.col >= width)
        {
            return(false);
        }
        return(true);
    }

    void create_contents() {
        for (int row = 0; row < height; row++) {
            vector<Square> row_vector = {};
            for (int col = 0; col < width; col++) {
                row_vector.push_back(Coords(row, col));
            }
            contents.push_back(row_vector);
        }
    }

    void create_animal(Species species, Coords coords=Coords()) {
        if (coords.row == -1) {
            coords.row = get_random_number(0, height-1);
        }
        if (coords.col == -1) {
            coords.col = get_random_number(0, width-1);
        }
        long id = animals_created;
        shared_ptr<Animal> new_animal = make_shared<Animal>(species, coords, id);
        
        animals_created++;
        animals_list.push_back(new_animal);
        contents[coords.row][coords.col].add_animal((new_animal));
    }

    void airdrop_animals(float capacity_fraction = 1) {
        float capacity = get_capacity();
        capacity *= capacity_fraction;
        for (Species species : species_list) {
            int num_to_airdrop = randomize_to_int(capacity / (species_defined * species.runrate));
            for (int i = 0; i < num_to_airdrop; i++) {
                create_animal(species);
            }
        }
    }

    void move_animal(shared_ptr<Animal> animal, Coords coords) {
        get_square_at_coords((*animal).coords).clear_animal_by_id((*animal).id);
        (*animal).coords = coords;
        get_square_at_coords((*animal).coords).add_animal(animal);
    }

    void kill_animal(shared_ptr<Animal> animal, int killer_id) {
        (*animal).alive = false;
        float amount_dropped = (*animal).reserves + (*animal).species.size;
        for (int i = 0; i < constants::num_foods; i++) {
            int food_amount_dropped = randomize_to_int(amount_dropped * constants::food_drop_rates[i] / constants::food_nutritions[i]);
            contents[(*animal).coords.row][(*animal).coords.col].foods_present[i] += food_amount_dropped;
        }

        if (killer_id == -1) {
            num_recently_starved++;
        }
        else {
            num_recently_killed++;
            recent_kills.push_back({ killer_id, (*animal).species.id });
        }
    }

    void breed_animal(shared_ptr<Animal> animal) {
        (*animal).reserves -= ((*animal).species.size * (1+constants::starting_reserves_ratio));
        create_animal((*animal).species, (*animal).coords);
    }

    Square get_square_at_coords(Coords coords) {
        return(contents[coords.row][coords.col]);
    }

    Square get_random_square() {
        return(Coords(get_random_number(0, height), get_random_number(0, width)));
    }

    SquareInfo get_square_info(shared_ptr<Animal> animal, Coords coords, bool is_within_1) {
        Square square = get_square_at_coords(coords);
        SquareInfo info = SquareInfo(coords);
        info.eligible_move = is_within_1;
        info.visible_threat = false;
        info.best_food = 0;
        info.largest_visible_prey = 0;
        info.largest_chaseable_prey = 0;
    
        Animal seen_animal_details;
        for (shared_ptr<Animal> seen_animal : square.animals_present) {
            seen_animal_details = *seen_animal;
            if (is_within_1 || !seen_animal_details.species.camo) {
                if (seen_animal_details.alive) {
                    if (predation_map[seen_animal_details.species.id][(*animal).species.id]) {
                        info.visible_threat = true;
                    }
                    if (predation_map[(*animal).species.id][seen_animal_details.species.id]) {
                        info.largest_visible_prey = max(info.largest_visible_prey, seen_animal_details.species.size);
                    }
                    if (pursuit_map[(*animal).species.id][seen_animal_details.species.id]) {
                        info.largest_chaseable_prey = max(info.largest_visible_prey, seen_animal_details.species.size);
                    }
                }
            }
        }
        if (is_within_1) {
            for (int food_id = 0; food_id < constants::num_foods; food_id++) {
                if ((*animal).species.edibles[food_id] && square.foods_present[food_id])
                    info.best_food = max(info.best_food, constants::food_nutritions[food_id]);
            }
        }

        return(info);
    }

    vector<SquareInfo> get_vision(shared_ptr<Animal> animal) {
        vector<SquareInfo> output = {};
        for (array<int, 2> m : constants::dist_01_neighbors) {
            Coords new_coords = (*animal).coords.add_move(m);
            if(is_on_map(new_coords))
                output.push_back(get_square_info(animal, new_coords, true));
        }
        for (array<int, 2> m : constants::dist_2_neighbors) {
            Coords new_coords = (*animal).coords.add_move(m);
            if (is_on_map(new_coords))
                output.push_back(get_square_info(animal, new_coords, false));
        }
        vector<Coords> visible_threats;
        for (const SquareInfo& temp_info : output) {
            if (temp_info.visible_threat) {
                visible_threats.push_back(temp_info.coords);
            }
        }

        for (SquareInfo &temp_info : output) {
            temp_info.distance_from_threat = temp_info.coords.closest_dist_to(visible_threats);
        }

        return(output);
    }

    set<int> get_active_speeds() {
        int round_no = rounds_elapsed % constants::max_speed;
        set<int> active_speeds;
        for (int speed = 1; speed <= constants::max_speed; speed++) {
            if (floor((speed * round_no) / constants::max_speed) < floor((speed * (round_no + 1)) / constants::max_speed)) {
                active_speeds.insert(speed);
            }
        }
        return(active_speeds);
    }

    vector<int> get_animal_counts() {
        vector<int> counts(species_defined, 0);

        for (shared_ptr<Animal> a : animals_list) {
            counts[(*a).species.id]++;
        }

        return(counts);
    }

    void sort_animals_by_initiative() {
        // several sorts that go in increasing order of importance
        shuffle(animals_list.begin(), animals_list.end(), rng);

        sort(begin(animals_list), end(animals_list), [](shared_ptr<Animal> a, shared_ptr<Animal> b) {
            return (*a).species.weapons < (*b).species.weapons;
        });

        sort(begin(animals_list), end(animals_list), [](shared_ptr<Animal> a, shared_ptr<Animal> b) {
            return (*a).species.size < (*b).species.size;
        });
        
        sort(begin(animals_list), end(animals_list), [](shared_ptr<Animal> a, shared_ptr<Animal> b) {
            return (*a).species.speed > (*b).species.speed;
        });
    }

    void perform_move(shared_ptr<Animal> animal) {
        vector<SquareInfo> vision = get_vision(animal);
        vector<SquareInfo> moves;
        for (const SquareInfo& si : vision) {
            if (si.eligible_move) {
                moves.push_back(si);
            }
        }

        // require best distance from threat.
        float best_threat_dist = 0;
        for (const SquareInfo& si : moves) {
            if (si.distance_from_threat > best_threat_dist) {
                best_threat_dist = si.distance_from_threat;
            }
        }
        best_threat_dist = min(best_threat_dist, float(2.1)); // we accept anything >2

        vector<SquareInfo> best_moves;
        for (const SquareInfo& si : moves) {
            if (si.distance_from_threat >= best_threat_dist) {
                best_moves.push_back(si);
            }
        }
        moves = best_moves;
        
        
        // at equality, require largest killable prey
        float best_prey = 0;
        for (const SquareInfo& si : moves) {
            if (si.largest_visible_prey > best_prey) {
                best_prey = si.largest_visible_prey;
            }
        }
        best_moves = {};
        for (const SquareInfo& si : moves) {
            if (si.largest_visible_prey >= best_prey) {
                best_moves.push_back(si);
            }
        }
        moves = best_moves;

        // at equality, require best food available
        float best_food = 0;
        for (const SquareInfo& si : moves) {
            if (si.best_food > best_food) {
                best_food = si.best_food;
            }
        }
        best_moves = {};
        for (const SquareInfo& si : moves) {
            if (si.best_food >= best_food) {
                best_moves.push_back(si);
            }
        }
        moves = best_moves;

        // if no food is available and multiple squares under consideration, look for chaseable prey.
        if (best_food == 0 && moves.size() > 1) {
            float best_chase = 0;
            for (const SquareInfo& si : vision) {
                if (si.largest_chaseable_prey > best_chase) {
                    best_prey = si.largest_chaseable_prey;
                }
            }
            vector<Coords> chase_locations;
            for (const SquareInfo& si : vision) {
                if (si.largest_chaseable_prey >= best_chase) {
                    chase_locations.push_back(si.coords);        
                }
            }
            
            float best_chase_dist = 999999;
            for (SquareInfo si : moves) {
                float chase_dist = si.coords.closest_dist_to(chase_locations);
                best_chase_dist = min(best_chase_dist, chase_dist);
            }

            vector<SquareInfo> best_moves;
            for (SquareInfo si : moves) {
                float chase_dist = si.coords.closest_dist_to(chase_locations);
                if (chase_dist <= best_chase_dist) {
                    best_moves.push_back(si);
                }
            }
            moves = best_moves;
        }
        move_animal(animal, moves[get_random_number(0, moves.size() - 1)].coords);
    }

    void perform_fight(shared_ptr<Animal> animal) {
        Square square = get_square_at_coords((*animal).coords);
        float largest_prey = 0;
        shared_ptr<Animal> to_eat;

        Animal target_details;
        for (shared_ptr<Animal> target : square.animals_present) {
            target_details = *target;
            if ((target_details.alive && (predation_map[(*animal).species.id][target_details.species.id])) && (target_details.species.size > largest_prey)) {
                largest_prey = target_details.species.size;
                to_eat = target;
            }
        }

        if (largest_prey > 0) {
            kill_animal(to_eat, (*animal).species.id);
        }
    }

    void perform_feed(shared_ptr<Animal> animal) {
        Square square = get_square_at_coords((*animal).coords);
        for (int food_id = 0; food_id < constants::num_foods; food_id++) {
            if ((square.foods_present[food_id] > 0) && ((*animal).species.edibles[food_id] == true)) {
                contents[(*animal).coords.row][(*animal).coords.col].foods_present[food_id]--;
                (*animal).reserves += constants::food_nutritions[food_id];
                return;
            }
        }
    }

    void perform_action(shared_ptr<Animal> animal) {
        if (!((*animal).alive)) {
            return;
        }

        perform_move(animal);
        perform_fight(animal);
        perform_feed(animal);
    }

    float get_capacity() {
        float capacity = 0;
        for (int i = 0; i < constants::num_foods; i++) {
            capacity += (constants::food_nutritions[i] * size * constants::food_spawn_rates[i]);
        }

        return(capacity);
    }

    int get_random_row() {
        return(get_random_number(0, height - 1));
    }
    int get_random_col() {
        return(get_random_number(0, width - 1));
    }

    void spawn_foods(array<float, constants::num_foods> spawn_rates) {
        for (int i = 0; i < constants::num_foods; i++) {
            int food_amount_spawned = randomize_to_int(size * spawn_rates[i]);
            for (int j = 0; j < food_amount_spawned; j++) {
                contents[get_random_row()][get_random_col()].foods_present[i]++;
            }
        }
    }

    void exec_growth() {
        spawn_foods(constants::food_spawn_rates);
    }

    void exec_actions() {
        sort_animals_by_initiative();
        set<int> speeds = get_active_speeds();
        for (shared_ptr<Animal> a : animals_list) {
            if (speeds.find((*a).species.speed) != speeds.end()) {
                perform_action(a);
                (*a).exec_hunger();
            }
        }
    }
    
    void exec_breeding_starvation() {
        vector<shared_ptr<Animal>> to_breed = {};
        for (shared_ptr<Animal> a : animals_list) {
            if ((*a).reserves <= 0) {
                kill_animal(a, -1);
            }
            if ((*a).reserves >= (constants::reproduction_reserves_ratio * (*a).species.size) ) {
                to_breed.push_back(a);
            }
        }
        for (shared_ptr<Animal> breeder : to_breed) {
            breed_animal(breeder);
        }
    }

    void clear_dead_animals() {
        vector<shared_ptr<Animal>> surviving_animals = {};
        for (shared_ptr<Animal> a : animals_list) {
            if ((*a).alive) {
                surviving_animals.push_back(a);
            }
            else {
                contents[(*a).coords.row][(*a).coords.col].clear_animal_by_id((*a).id);
            }
        }
        animals_list = surviving_animals;
    }

    void exec_round() {
        exec_growth();
        exec_actions();
        clear_dead_animals();
        exec_breeding_starvation();
        clear_dead_animals();
        rounds_elapsed++;
        if (rounds_elapsed % 500 == 0) {
            print_status();
        }
    }

    array<float, constants::num_foods> food_abundances() {
        array<float, constants::num_foods> abundances;
        abundances.fill(0);
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                for(int food_id = 0;food_id < constants::num_foods; food_id++) {
                    abundances[food_id] += contents[row][col].foods_present[food_id];
                }
            }
        }
        for (int food_id = 0; food_id < constants::num_foods; food_id++) {
            abundances[food_id] /= size;
        }
        return(abundances);
    }

    void print_status() {
        auto new_update = chrono::system_clock::now();
        cout << "\n\n\nStatus after round " << rounds_elapsed << ", " << chrono::duration_cast<chrono::seconds>(new_update-latest_update).count() << "s elapsed since last update:\n";
        vector<int> counts = get_animal_counts();
        float runrate_used = 0;
        int total_animals = 0;
        for (int i = 0; i < species_defined; i++) {
            runrate_used += (species_list[i].runrate * counts[i]);
            total_animals += counts[i];
        }
        cout << total_animals << " alive, current metabolic runrate is at " << (runrate_used * 100)/get_capacity() << "% of capacity\n";

        for (int i = 0; i < species_defined; i++) {
            if (counts[i] >= 10) {
                cout << species_list[i].print_details_string() << " has population " << counts[i] << "\n";
            }
        }
        auto abundances = food_abundances();
        for (int i = 0; i < constants::num_foods; i++) {
            cout << "Current abundance of " << constants::food_names[i] << " is " << abundances[i] << "\n";
        }
        cout << "Since the last update " << num_recently_killed << " animals have been killed and " << num_recently_starved << " have starved\n";
        if (num_recently_killed > 0) {
            cout << "Sample recent kills:\n";
            int num_to_display = min(5,num_recently_killed);
            for (int i = 0; i < num_to_display; i++) {
                cout << recent_kills[i][0] << " killed " << recent_kills[i][1] << "\n";
            }
        }

        num_recently_killed = 0;
        num_recently_starved = 0;
        recent_kills = {};
        latest_update = new_update;
    }
};

int main()
{
    World my_world = World(500, 500);
    my_world.print_status();

    for (int i = 0; i < 100000; i++)
        my_world.exec_round();
        
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu
