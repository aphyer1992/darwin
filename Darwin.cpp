
// Darwin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <array>
#include <random>
#include <set>
#include <map>
#include <list>
#include <unordered_map> 
#include <memory>
#include <chrono>
#include <ctime>

#undef INFINITY
using namespace std;

namespace DA
{
    constexpr double INFINITY = 1e29;
    template<class T_> constexpr T_ Square(const T_& x) { return x * x; }
    template<class C_, class P_> void Sort(C_* vals, const P_& pred)
    {
        sort(vals->begin(), vals->end(), pred);
    }

    template<class C_, class P_> void Filter(C_* c, P_ pred)
    {
        typedef decltype(*c->begin()) element_t;
        auto stop = remove_if(c->begin(), c->end(), [&](const element_t& e) {return !pred(e); });
        c->erase(stop, c->end());
    }
    // specialization for map
    template<class K_, class V_, class P_> void Filter(unordered_map<K_, V_>* c, P_ pred)
    {
        for (auto p = c->begin(); p != c->end(); )
        {
            if (pred(*p))
                ++p;   // keep
            else
                p = c->erase(p);
        }
    }
    template<class K_, class V_, class P_> void Filter(map<K_, V_>* c, P_ pred)
    {
        for (auto p = c->begin(); p != c->end(); )
        {
            if (pred(*p))
                ++p;   // keep
            else
                p = c->erase(p);
        }
    }
    // and for list
    template<class E_, class P_> void Filter(list<E_>* c, P_ pred)
    {
        erase_if(*c, [&](const E_& x) { return !pred(x); });
    }

    template<class I_, class F_>
    I_ Most(I_ begin, I_ end, F_ func)
    {
        auto q = begin;
        if (q == end)
            return q;
        auto best = func(*begin);
        while (++begin != end)
        {
            if (auto t = func(*begin); t > best)
            {
                q = begin;
                best = t;
            }
        }
        return q;
    }

    template<class C_, class F_>
    auto Most(const C_& c, F_ func)
    {
        return Most(c.begin(), c.end(), func);
    }


    template<class I_, class F_>
    auto MostVal(I_ begin, I_ end, F_ func)
    {
        return func(*Most(begin, end, func));
    }

    template<class C_, class F_>
    auto MostVal(const C_& c, F_ func)
    {
        return func(*Most(c.begin(), c.end(), func));
    }
}

using move_t = array<int, 2>;

namespace constants
{
    constexpr double meat_nutrition = 1;
    constexpr double bone_nutrition = 0.5;
    constexpr double offal_nutrition = 0.25;
    constexpr double fruit_nutrition = 0.7;
    constexpr double grass_nutrition = 0.3;
    constexpr double seeds_nutrition = 0.15;

    constexpr int num_foods = 6;
    // IMPORTANT: MUST BE SORTED DESCENDING BY NUTRITION!
    const string food_names[num_foods] = { "Meat", "Fruit", "Bone", "Grass", "Offal", "Seeds" };
    constexpr array<double, num_foods> food_nutritions = { meat_nutrition, fruit_nutrition, bone_nutrition, grass_nutrition, offal_nutrition, seeds_nutrition };
    constexpr array<double, num_foods> food_costs = { 1, 3, 4, 5, 2, 1 };
    constexpr array<double, num_foods> food_spawn_rates = { 0, 0.0002, 0, 0.002, 0, 0.001 };
    constexpr array<double, num_foods> food_drop_rates = { 0.5, 0, 0.25, 0, 0.25, 0 };
    constexpr array<double, num_foods> food_starting_rates = { 0.01, 0.05, 0.10, 0.24, 0.17, 0.13 };

    constexpr int max_speed = 10;
    constexpr int max_size = 1000;
    constexpr int max_weapons_armor = 10;

    constexpr int num_species = 300;

    constexpr double starting_reserves_ratio = 0.5;
    constexpr double reproduction_reserves_ratio = 2.0;
    constexpr double metabolic_rate = 0.01;

    constexpr array<array<int, 2>, 5> dist_01_neighbors = { { {0,0}, {0,1}, {0,-1}, {1,0}, {-1,0} } }; // why do i need a third brace?
    constexpr array<array<int, 2>, 8> dist_2_neighbors = { { {0,2}, {0,-2}, {2,0}, {-2,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1} } };

}

int triangle(int n)
{
    return (n * (n + 1)) / 2;
}

double calc_size(int weapons, int armor, int speed, array<bool, 6> edibles, bool camo)
{
    double size = triangle(weapons) + 0.5 * (triangle(armor) + triangle(speed));

    for (int i = 0; i < constants::num_foods; i++) {
        size += edibles[i] ? constants::food_costs[i] : 0;
    }

    size *= camo ? 2 : 1;
    return size;
}

struct Coords
{
    int row = -1;
    int col = -1;

    Coords& operator+=(move_t inc) {
        row += inc[0];
        col += inc[1];
        return *this;
    }

    double dist_from(const Coords& other) const
    {
        return sqrt(DA::Square(row - other.row) + DA::Square(col - other.col));
    }

    double closest_dist_to(const vector<Coords>& targets) const
    {
        double dist = DA::INFINITY; // I tried making this INT_MAX and it blew up and I don't understand why
        for (const Coords& target : targets) {
            dist = min(dist, dist_from(target));
        }
        return(dist);
    }
};

Coords operator+(const Coords& lhs, move_t rhs)
{
    return Coords(lhs.row + rhs[0], lhs.col + rhs[1]);
}



struct Species
{
    string name_;
    double size_, runrate_, initiative_;
    int weapons_, armor_, speed_;
    int id_;
    array<bool, 6> edibles_;
    bool camo_;

    string default_name()
    {
        string default_name = "W" + to_string(weapons_) + "A" + to_string(armor_) + "S" + to_string(speed_);
        if (camo_)
        {
            default_name += " Camo";
        }
        default_name += " Eats";
        for (int i = 0; i < constants::num_foods; i++) {
            if (edibles_[i]) {
                default_name += " " + constants::food_names[i];
            }
        }
        return default_name;
    }

    Species() {}

    Species(int weapons, int armor, int speed, array<bool, 6> edibles, bool camo, string name = "")
        : name_(name)
        , weapons_(weapons)
        , armor_(armor)
        , speed_(speed)
        , edibles_(edibles)
        , camo_(camo)
    {
        size_ = calc_size(weapons, armor, speed, edibles, camo);
        runrate_ = size_ * speed * constants::metabolic_rate / constants::max_speed;
        initiative_ = -weapons_ + constants::max_weapons_armor * (-size_ + constants::max_size * speed_);

        if (name_.empty())
            name_ = default_name();
    }
};

ostream& operator<<(ostream& dst, Species s)
{
    dst << s.name_ << " (" << to_string(s.id_) << ") : " << s.default_name();
    return dst;
}

class Animal
{
public:
    const Species* species_;
    Coords coords_;
    double reserves_;
    long id_;
    bool alive_;

    Animal() { }

    void Reset(const Species* species, Coords coords, long id)
    {
        species_ = species;
        coords_ = coords;
        id_ = id;
        reserves_ = species->size_ * constants::starting_reserves_ratio;
        alive_ = true;
    }

    Animal(const Species* species, Coords coords, long id)
        : species_(species)
        , coords_(coords)
        , reserves_(species->size_* constants::starting_reserves_ratio)
        , id_(id)
        , alive_(true)
    {   }

    void exec_hunger() {
        reserves_ -= constants::metabolic_rate * species_->size_;
    }

    void print(ostream& dst) const {
        dst << "\nAnimal " << id_ << " Info:\n";
        dst << "Species: " << species_ << "\n";
        dst << "Reserves: " << reserves_ << "\n";
        dst << "Alive: " << alive_ << "\n";
        dst << "Located at: (" << coords_.row << "," << coords_.col << ")\n\n";
    }
};


ostream& operator<<(ostream& dst, const Animal& animal)
{
    animal.print(dst);
    return dst;
}

class Square {
public:
    array<int, constants::num_foods> foods_present;
    list<shared_ptr<Animal>> animals_present;

    Square() {
        foods_present.fill(0);
        animals_present = {};
    }

    void add_animal(shared_ptr<Animal> to_add)
    {
        auto p = find_if(animals_present.begin(), animals_present.end(), [](const auto& a) { return !a->alive_; });
        if (p == animals_present.end())
            animals_present.push_back(to_add);
        else
            *p = to_add;
    }

    void clear_dead_animals() {
        DA::Filter(&animals_present, [&](auto a) { return a->alive_; });
    }

    inline void clear_animal_by_id(long id) {
        auto p = find_if(animals_present.begin(), animals_present.end(), [&](auto a) { return a->id_ == id; });
        // this will fail if we try to clear an animal that isn't here
        animals_present.erase(p);
    }
};

struct SquareInfo
{
    double best_food;
    double largest_visible_prey;
    double largest_chaseable_prey;
    double distance_from_threat;
    Coords coords_;
    bool eligible_move;
    bool visible_threat;

    SquareInfo(Coords coords)
        : coords_(coords)
    { }
};

struct Workspace_
{
    vector<SquareInfo> moves_;
    vector<Coords> chase_;

    void Reset()
    {
        moves_.clear();
        chase_.clear();
    }
};

bool Quicker1(const shared_ptr<Animal>& lhs, const shared_ptr<Animal>& rhs)
{
    if (lhs->species_->speed_ != rhs->species_->speed_)
        return lhs->species_->speed_ > rhs->species_->speed_;
    if (lhs->species_->size_ != rhs->species_->size_)
        return lhs->species_->size_ < rhs->species_->size_;
    return lhs->species_->weapons_ < rhs->species_->weapons_;
}


double Quickness(const Species& s)
{
    return -s.weapons_ + constants::max_weapons_armor * (-s.size_ + constants::max_size * s.speed_);
}

bool Quicker2(const shared_ptr<Animal>& lhs, const shared_ptr<Animal>& rhs)
{
    return lhs->species_->initiative_ > rhs->species_->initiative_;
}

constexpr array<array<bool, constants::max_speed + 1>, constants::max_speed> make_active_speeds()
{
    array<array<bool, constants::max_speed + 1>, constants::max_speed> retval;
    for (int round_no = 0; round_no < constants::max_speed; ++round_no)
    {
        for (int speed = 0; speed < constants::max_speed; speed++)
            retval[round_no][speed] = ((speed * round_no) % constants::max_speed) + speed >= constants::max_speed;
        retval[round_no][constants::max_speed] = true;
    }
    return retval;
}

constexpr array<array<bool, constants::max_speed + 1>, constants::max_speed> active_speeds = make_active_speeds();

class World {
public:
    int width;
    int height;
    int size;
    int species_defined;
    long animals_created;
    int rounds_elapsed;
    array<Species, constants::num_species> species_list;
    array<array<bool, constants::num_species>, constants::num_species> predation_map;
    array<array<bool, constants::num_species>, constants::num_species> pursuit_map;
    mt19937 rng;
    vector<vector<Square>> contents;
    vector<shared_ptr<Animal>> animals_list, recycle;
    chrono::system_clock::time_point latest_update;
    int num_recently_killed;
    int num_recently_starved;


    World(int width_in, int height_in)
        : width(width_in)
        , height(height_in)
        , size(width* height)
        , species_defined(0)
        , animals_created(0)
        , rounds_elapsed(0)
    {
        setup_fixed_species();
        setup_random_species();
        sort_species_by_size();
        create_predation_maps();
        create_contents();
        spawn_foods(constants::food_starting_rates);
        airdrop_animals();
        latest_update = chrono::system_clock::now();
    };

    bool random_bool(double prob)
    {
        return uniform_real_distribution<>(0.0, 1.0)(rng) < prob;
    }

    int random_int(int min, int max) {
        uniform_int_distribution<> dis(min, max);
        return(dis(rng));
    }

    int randomize_to_int(double input) {
        int out = static_cast<int>(input);
        if (out + uniform_real_distribution<>(0.0, 1.0)(rng) < input)
            ++out;
        return(out);
    }

    void add_species(int weapons, int armor, int speed, array<bool, 6> edibles, bool camo, string name = "")
    {
        Species new_species(weapons, armor, speed, edibles, camo, name);
        species_list[species_defined] = new_species;
        ++species_defined;
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
            array<bool, 6> temp_edibles;
            temp_edibles.fill(false);
            temp_edibles[i] = true;
            setup_fixed_herbivores(temp_edibles, constants::food_names[i]);
        }
        constexpr array<bool, 6> multiplant_edibles = { false, true, false, true, false, true };
        setup_fixed_herbivores(multiplant_edibles, "Multiplant");

        constexpr array <bool, 6> meat_only = { true, false, false, false, false, false };
        for (int i = 1; i < 10; i++)
        {
            add_species(i, 0, 1, meat_only, true, "Lurker " + to_string(i));
        }

        for (int i = 0; i < constants::num_foods; i++)
        {
            array<bool, 6> temp_edibles = { true,false,false,false,false };
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
            int weapons = random_int(-2, constants::max_weapons_armor);
            weapons = max(0, weapons);
            int armor = random_int(0, constants::max_weapons_armor);
            armor = armor - weapons;
            armor = max(0, armor);
            int speed = random_int(1, constants::max_speed);
            bool camo = random_bool(1.0 / 3.0);
            array<bool, 6> edibles;
            for (bool& e : edibles)
            {
                e = random_bool(1.0 / 3.0);
            }
            add_species(weapons, armor, speed, edibles, camo, "Random Animal");
        }
    }

    void sort_species_by_size()
    {
        auto by_size = [](const Species& lhs, const Species& rhs) { return lhs.size_ < rhs.size_; };

        DA::Sort(&species_list, by_size);
        for (int i = 0; i < constants::num_species; i++) {
            species_list[i].id_ = i;
        }
    }

    void create_predation_maps()
    {
        cout << "Creating Predation Maps\n";

        for (int a = 0; a < constants::num_species; a++)
        {
            const Species& s_a = species_list[a];
            for (int b = 0; b < constants::num_species; b++)
            {
                const Species& s_b = species_list[b];
                predation_map[a][b] = s_a.weapons_ > s_b.weapons_ + s_b.armor_;
                pursuit_map[a][b] = predation_map[a][b] && s_a.speed_ > s_b.speed_;
            }
        }
    }

    inline bool is_on_map(Coords coords) {
        return coords.row >= 0 && coords.row < height && coords.col >= 0 && coords.col < width;
    }

    void create_contents() {
        for (int row = 0; row < height; row++) {
            contents.push_back(vector<Square>(width));
        }
    }

    void create_animal(const Species* species, Coords coords = Coords()) {
        if (coords.row == -1) {
            coords.row = random_int(0, height - 1);
        }
        if (coords.col == -1) {
            coords.col = random_int(0, width - 1);
        }
        long id = animals_created;
        shared_ptr<Animal> new_animal;
        if (recycle.empty())
            new_animal = make_shared<Animal>(species, coords, id);
        else
        {
            new_animal = recycle.back();
            new_animal->Reset(species, coords, id);
            recycle.pop_back();
        }

        animals_created++;
        animals_list.push_back(new_animal);
        contents[coords.row][coords.col].add_animal(new_animal);
    }

    void airdrop_animals(double capacity_fraction = 1) {
        double capacity = get_capacity() * capacity_fraction;
        for (const Species& species : species_list) {
            int num_to_airdrop = randomize_to_int(capacity / (species_defined * species.runrate_));
            for (int i = 0; i < num_to_airdrop; i++) {
                create_animal(&species);
            }
        }
    }

    inline Square& get_square_at_coords(Coords coords) {
        return contents[coords.row][coords.col];
    }

    void move_animal(shared_ptr<Animal> animal, Coords coords) {
        get_square_at_coords(animal->coords_).clear_animal_by_id(animal->id_);
        animal->coords_ = coords;
        get_square_at_coords(animal->coords_).add_animal(animal);
    }

    void kill_animal(shared_ptr<Animal> animal, int killer_id) {
        animal->alive_ = false;
        double amount_dropped = animal->reserves_ + animal->species_->size_;
        for (int i = 0; i < constants::num_foods; i++) {
            int food_amount_dropped = randomize_to_int(amount_dropped * constants::food_drop_rates[i] / constants::food_nutritions[i]);
            contents[animal->coords_.row][animal->coords_.col].foods_present[i] += food_amount_dropped;
        }

        if (killer_id == -1) {
            num_recently_starved++;
        }
        else {
            num_recently_killed++;
        }
    }

    void breed_animal(shared_ptr<Animal> animal) {
        animal->reserves_ -= animal->species_->size_ * (1 + constants::starting_reserves_ratio);
        create_animal(animal->species_, animal->coords_);
    }

    SquareInfo get_square_info(shared_ptr<Animal> animal, Coords coords, bool is_within_1) {
        Square& square = get_square_at_coords(coords);
        SquareInfo info(coords);
        info.eligible_move = is_within_1;
        info.visible_threat = false;
        info.largest_visible_prey = 0;
        info.largest_chaseable_prey = 0;

        for (const auto& seen_animal : square.animals_present)
        {
            if (!seen_animal->alive_)
                continue;
            if (is_within_1 || !seen_animal->species_->camo_)
            {
                if (predation_map[seen_animal->species_->id_][animal->species_->id_]) {
                    info.visible_threat = true;
                }
                if (predation_map[animal->species_->id_][seen_animal->species_->id_]) {
                    info.largest_visible_prey = max(info.largest_visible_prey, seen_animal->species_->size_);
                    if (pursuit_map[animal->species_->id_][seen_animal->species_->id_]) {
                        info.largest_chaseable_prey = max(info.largest_visible_prey, seen_animal->species_->size_);
                    }
                }
            }
        }

        return info;
    }

    void get_vision(shared_ptr<Animal> animal, vector<SquareInfo>* vision)
    {
        for (const array<int, 2>&m : constants::dist_01_neighbors) {
            Coords new_coords = animal->coords_ + m;
            if (is_on_map(new_coords))
                vision->push_back(get_square_info(animal, new_coords, true));
        }
        for (const array<int, 2>&m : constants::dist_2_neighbors) {
            Coords new_coords = animal->coords_ + m;
            if (is_on_map(new_coords))
                vision->push_back(get_square_info(animal, new_coords, false));
        }
        vector<Coords> visible_threats;
        for (const SquareInfo& temp_info : *vision) {
            if (temp_info.visible_threat) {
                visible_threats.push_back(temp_info.coords_);
            }
        }

        for (SquareInfo& temp_info : *vision) {
            temp_info.distance_from_threat = temp_info.coords_.closest_dist_to(visible_threats);
        }
    }

    vector<int> animal_counts()
    {
        vector<int> counts(species_defined, 0);
        for (const shared_ptr<Animal>& a : animals_list)
            counts[a->species_->id_]++;
        return counts;
    }

    void sort_animals_by_initiative() {
        // several sorts that go in increasing order of importance
        shuffle(animals_list.begin(), animals_list.end(), rng);
        DA::Sort(&animals_list, Quicker2);
    }

    void perform_move(shared_ptr<Animal> animal, Workspace_& work)
    {
        get_vision(animal, &work.moves_);
        auto start = work.moves_.begin(), end = work.moves_.end();

        auto stop = remove_if(start, end, [](const SquareInfo& si) { return !si.eligible_move; });
        // require best distance from threat.
        if (stop > start + 1)
        {
            auto pBest = DA::Most(start, stop, [](const SquareInfo& si) { return si.distance_from_threat; });
            double best_threat_dist = min(pBest->distance_from_threat, double(2.1)); // we accept anything >2

            stop = remove_if(start, stop, [=](const SquareInfo& si) { return si.distance_from_threat < best_threat_dist; });

            // at equality, require largest killable prey
            if (stop > start + 1)
            {
                double bestPrey = DA::MostVal(start, stop, [](const SquareInfo& si) { return si.largest_visible_prey; });
                stop = remove_if(start, stop, [=](const SquareInfo& si) { return si.largest_visible_prey < bestPrey; });

                // at equality, require best food available
                if (stop > start + 1)
                {
                    double bestFood = DA::MostVal(start, stop, [](const SquareInfo& si) { return si.best_food; });
                    stop = remove_if(start, stop, [=](const SquareInfo& si) { return si.best_food < bestFood; });

                    // if no food is available and multiple squares under consideration, look for chaseable prey.
                    if (stop > start + 1 && start->best_food == 0)
                    {
                        double bestPrey = DA::MostVal(start, stop, [](const SquareInfo& si) { return si.largest_chaseable_prey; });

                        for (const SquareInfo& si : work.moves_) {
                            if (si.largest_chaseable_prey >= bestPrey) {
                                work.chase_.push_back(si.coords_);
                            }
                        }

                        auto negDist = [&](const SquareInfo& si) { return -si.coords_.closest_dist_to(work.chase_); };
                        double minDist = -DA::MostVal(start, stop, negDist);
                        stop = remove_if(start, stop, [&](const SquareInfo& si) { return -negDist(si) > minDist; });
                    }
                }
            }
        }
        auto pMove = start;
        if (stop > start + 1)
            pMove += random_int(0, static_cast<int>(stop - start) - 1);
        move_animal(animal, pMove->coords_);
    }

    void perform_fight(shared_ptr<Animal> animal)
    {
        const Square& square = get_square_at_coords(animal->coords_);
        shared_ptr<Animal> prey;

        for (const auto& target : square.animals_present)
        {
            if (target->alive_ && predation_map[animal->species_->id_][target->species_->id_])
            {
                if (!prey || target->species_->size_ > prey->species_->size_)
                    prey = target;
            }
        }

        if (prey)
            kill_animal(prey, animal->species_->id_);
    }

    void perform_feed(shared_ptr<Animal> animal)
    {
        const Square& square = get_square_at_coords(animal->coords_);
        for (int food_id = 0; food_id < constants::num_foods; food_id++) {
            if ((square.foods_present[food_id] > 0) && (animal->species_->edibles_[food_id] == true)) {
                contents[animal->coords_.row][animal->coords_.col].foods_present[food_id]--;
                animal->reserves_ += constants::food_nutritions[food_id];
                return;
            }
        }
    }

    void perform_action(const shared_ptr<Animal>& animal, Workspace_& work)
    {
        if (animal->alive_)
        {
            perform_move(animal, work);
            perform_fight(animal);
            perform_feed(animal);
        }
    }

    double get_capacity()
    {
        double capacity = 0;
        for (int i = 0; i < constants::num_foods; i++)
            capacity += constants::food_nutritions[i] * size * constants::food_spawn_rates[i];
        return capacity;
    }

    int get_random_row() {
        return(random_int(0, height - 1));
    }
    int get_random_col() {
        return(random_int(0, width - 1));
    }

    void spawn_foods(array<double, constants::num_foods> spawn_rates) {
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
        const auto& active = active_speeds[rounds_elapsed % constants::max_speed];
        Workspace_ work;
        for (shared_ptr<Animal>& a : animals_list)
        {
            if (active[a->species_->speed_])
            {
                work.Reset();
                perform_action(a, work);
                a->exec_hunger();
            }
        }
    }

    void exec_breeding_starvation()
    {
        size_t n = animals_list.size();
        for (size_t ia = 0; ia < n; ++ia)
        {
            shared_ptr<Animal>& a = animals_list[ia];
            if (a->reserves_ <= 0)
                kill_animal(a, -1);
            else if (a->reserves_ >= constants::reproduction_reserves_ratio * a->species_->size_)
                breed_animal(a);
        }
    }

    void clear_dead_animals()
    {
        auto stop = remove_if(animals_list.begin(), animals_list.end(), [&](auto& e) {return !e->alive_; });
        auto endExisting = remove_if(stop, animals_list.end(), [&](auto& e) {return !e; });
        auto nrOld = recycle.size();
        recycle.insert(recycle.end(), stop, endExisting);
        animals_list.erase(stop, animals_list.end());

        // mark which squares contain recycled animals
        vector<char> marked(height * width, false);
        for (auto ii = nrOld; ii < recycle.size(); ++ii)
            marked[recycle[ii]->coords_.row + height * recycle[ii]->coords_.col] = '1';

        for (int iRow = 0; iRow < height; ++iRow)
            for (int iCol = 0; iCol < width; ++iCol)
                if (marked[iRow + height * iCol])
                    contents[iRow][iCol].clear_dead_animals();
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
        if (rounds_elapsed < 1e5) {
            airdrop_animals(1e-5);
        }
    }

    array<double, constants::num_foods> food_abundances() {
        array<double, constants::num_foods> abundances;
        abundances.fill(0);
        for (int row = 0; row < height; row++) {
            for (int col = 0; col < width; col++) {
                for (int food_id = 0; food_id < constants::num_foods; food_id++) {
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
        cout << "\n\n\nStatus after round " << rounds_elapsed << ", " << chrono::duration_cast<chrono::seconds>(new_update - latest_update).count() << "s elapsed since last update:\n";
        vector<int> counts = animal_counts();
        double runrate_used = 0;
        int total_animals = 0;
        for (int i = 0; i < species_defined; i++) {
            runrate_used += (species_list[i].runrate_ * counts[i]);
            total_animals += counts[i];
        }
        cout << total_animals << " alive, current metabolic runrate is at " << (runrate_used * 100) / get_capacity() << "% of capacity\n";

        for (int i = 0; i < species_defined; i++) {
            if (counts[i] >= 5) {
                cout << species_list[i] << " has population " << counts[i] << "\n";
            }
        }
        auto abundances = food_abundances();
        for (int i = 0; i < constants::num_foods; i++) {
            cout << "Current abundance of " << constants::food_names[i] << " is " << abundances[i] << "\n";
        }
        cout << "Since the last update " << num_recently_killed << " animals have been killed"
            " and " << num_recently_starved << " have starved\n";
        latest_update = new_update;
        num_recently_killed = 0;
        num_recently_starved = 0;
    }
};

int main()
{
    World my_world = World(500, 500);
    my_world.print_status();
    auto t1 = chrono::high_resolution_clock::now();

    for (int i = 0; i < 100000; i++)
        my_world.exec_round();

    auto t2 = chrono::high_resolution_clock::now();
    cout << "simulation took "
        << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count()
        << " milliseconds\n";

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu