#include "network.h"


//setup input ids and output ids. These are the same across all networks
Network::Network(int* idc)
{
    id_count = idc;
    for (int i = 0; i < 18; i++)
    {
        neurons[i] = Neuron();
        inputs_ids[i] = i;
    }
    for (int i = 18; i < 22; i++)
    {
        neurons[i] = Neuron();
        output_ids[i - 18] = i;
    }

    /* neurons[40] = Neuron();
     neurons[41] = Neuron();
     neurons[42] = Neuron();
     neurons[43] = Neuron();
     neurons[44] = Neuron();
     neurons[45] = Neuron();*/
};

void Network::add_link(map<tuple<Mutations, int, int>, int>& mut_hist)
{
    //find from and to make sure no repeats or opposites of links that already exist and no loops
    vector<int> ids;
    for (auto [key, val] : neurons)
    {
        if (count(begin(output_ids), end(output_ids), key) == 0) // no outputs for now
        {
            ids.push_back(key);
        }
    }

    //cout << "1: " << ids.size() << "\n";

    int from_id = ids[GetRandomValue(0, ids.size() - 1)];

    ids.clear();
    for (auto [key, val] : neurons)
    {
        if (count(begin(inputs_ids), end(inputs_ids), key) == 0) // no inputs for now
        {
            ids.push_back(key);
        }
    }

    vector<int> filtered_ids;

    //cout << "2: " << ids.size() << "\n";

    for (auto id : ids)
    {
        bool allowed = true;
        for (auto& link : links)
        {
            if ((from_id == link.from_id && id == link.to_id) || (from_id == link.to_id && id == link.from_id))
            {
                allowed = false;
                break;
            }
        }

        if (allowed)
        {
            filtered_ids.push_back(id);
        }
    }

    //cout << "2: " << filtered_ids.size() << "\n";

    // if there are no options to go to from of that node, start again
    if (filtered_ids.size() == 0)
    {
        add_link(mut_hist);
        return;
    }


    int to_id = filtered_ids[GetRandomValue(0, filtered_ids.size() - 1)];

    // if a cycle is formed, start again you cant topologically sort if theres a cycle
    if (cyclical(from_id, to_id))
    {
        cout << "CYCLIC" << endl;
        add_link(mut_hist);
        return;
    }

    tuple<Mutations, int, int> t = make_tuple(Mutations::ADD_LINK, from_id, to_id);

    Link link = Link();
    link.from_id = from_id;
    link.to_id = to_id;

    //source a random weight from the normal distribution
    random_device rd{};
    mt19937 gen{ rd() };

    std::normal_distribution<float> d{ 0.0, 0.1 };
    link.weight = d(gen);

    //make sure if the mutation has already occured, they have the same innovation number
    if (mut_hist.contains(t))
    {
        link.id = mut_hist[t];
    }
    else
    {
        mut_hist[t] = *id_count;
        link.id = *id_count;
        (*id_count)++;
    }

    links.push_back(link);
}

//if you can reach the node the link comes out of from the node the link is going into, there will be a cycle if the link is added
bool Network::cyclical(int original, int current)
{
    vector<int> dfs_stack;
    unordered_set<int> visited;
    dfs_stack.push_back(current);

    while (!dfs_stack.empty())
    {
        int current = dfs_stack[dfs_stack.size() - 1];
        dfs_stack.pop_back();

        if (current == original)
        {
            return true;
        }


        // Push neighbors onto the stack for exploration
        for (auto& link : links)
        {
            if (link.from_id == current && !visited.contains(link.to_id))
            {
                visited.insert(current);
                dfs_stack.push_back(link.to_id);
            }
        }
    }
    return false;
}

void Network::add_neuron(map<tuple<Mutations, int, int>, int>& mut_hist)
{
    //neuron can only be created by splitting an enabled link
    vector<int> enabled_link_indices;
    for (int i = 0; i < links.size(); i++)
    {
        if (!links[i].disabled)
        {
            enabled_link_indices.push_back(i);
        }
    }

    if (enabled_link_indices.size() == 0)
    {
        add_link(mut_hist);
        return;
    }

    int number = GetRandomValue(0, enabled_link_indices.size() - 1);

    int index = enabled_link_indices[number];
    links[index].disabled = true; // disable old link

    tuple<Mutations, int, int> t = make_tuple(Mutations::ADD_NEURON, links[index].from_id, links[index].to_id);

    //make sure if the mutation has already occured, all new structures have the same innovation numbers
    if (mut_hist.contains(t))
    {
        int neuron_id = mut_hist[t];//new neuron between
        neurons[neuron_id] = Neuron();

        Link link1 = Link(); //new link from old from to new neuron
        link1.from_id = links[index].from_id;
        link1.to_id = neuron_id;
        link1.id = neuron_id + 1;
        link1.weight = 1.0;
        links.push_back(link1);

        Link link2 = Link(); //new link from new neuron to old to
        link2.from_id = neuron_id;
        link2.to_id = links[index].to_id;
        link2.weight = links[index].weight;
        link2.id = neuron_id + 2;
        links.push_back(link2);
    }
    else
    {
        int neuron_id = *id_count; //new neuron between
        neurons[neuron_id] = Neuron();
        (*id_count)++;

        mut_hist[t] = neuron_id;


        Link link1 = Link(); //new link from old from to new neuron
        link1.from_id = links[index].from_id;
        link1.to_id = neuron_id;
        link1.weight = 1.0;
        link1.id = *id_count;
        (*id_count)++;
        links.push_back(link1);



        Link link2 = Link(); //new link from new neuron to old to
        link2.from_id = neuron_id;
        link2.to_id = links[index].to_id;
        link2.weight = links[index].weight;
        link2.id = *id_count;
        (*id_count)++;
        links.push_back(link2);
    }
}

void Network::change_weight()
{
    if (links.size() == 0)
    {
        return;
    }

    //sample the change in value from the normal distribution
    random_device rd{};
    mt19937 gen{ rd() };

    std::normal_distribution<float> d{ 0.0, 0.2 }; //TODO make sigma start at 0.4 and go to 0.1 or even less later
    int index = GetRandomValue(0, links.size() - 1);
    links[index].weight += d(gen);
}

void Network::reset_weight()
{
    if (links.size() == 0)
    {
        return;
    }

    links[GetRandomValue(0, links.size() - 1)].weight = (float)GetRandomValue(-1000000, 1000000) / 1000000.0f;
}

void Network::toggle_link()
{
    if (links.size() == 0)
    {
        return;
    }

    Link link = links[GetRandomValue(0, links.size() - 1)];
    link.disabled = !link.disabled;
}

// do a topological sort by repeatedly doing dfs on each node, starting with the nodes with the least number of links going from it
void Network::calc_output() 
{
    //TODO MAKE IT SO IT ONLY LOOKS FOR ONES THAT AFFECT THE OUTPUT LAYER
    vector<int> stack; // end order in which to visit
    map<int, vector<int>> neighbours; // key is id of neuron, value is list neurons you can go to from there
    map<int, bool> visited; // whether a neuron of id int has been visited

    //initialise maps
    for (auto [id, _] : neurons)
    {
        visited[id] = false;
        neighbours[id] = {};
    }

    //fill neighbours map
    for (auto& link : links)
    {
        neighbours[link.from_id].push_back(link.to_id);
    }

    //create a list of vectors with the first in the pair being the id of a node, and the second being how many links go from it
    vector<pair<int, int>> pairs;
    for (auto [key, val] : neighbours)
    {
        pairs.push_back({ key, val.size() });
    }

    //sort it so it's from smallest to largest so we do dfs on them first
    sort(pairs.begin(), pairs.end(), [](auto& a, auto& b) {
        return a.second < b.second;
        });

    // locations in order of least outward edges to most
    vector<int> to_visit; 

    for (auto [first, _] : pairs)
    {
        to_visit.push_back(first);
    }

    //do dfs in the order
    for (auto id : to_visit)
    {
        if (visited[id])
        {
            continue;
        }
        dfs(stack, neighbours, visited, id);
    }

    //compute activation of each neuron based on previous neurons. Order is calculated stack.
    //formula is activation_function(sum_of(link_weights * their_corresponding_neuron_activations))
    for (auto id : stack)
    {
        //cout << id << endl;
        if (count(begin(inputs_ids), end(inputs_ids), id))
        {
            continue;
        }

        float sum = 0.0;
        for (auto &link : links)
        {
            if (link.to_id == id && !link.disabled)
            {
                sum += neurons[link.from_id].activation * link.weight;
            }
        }
        neurons[id].activation = sigmoid(sum);
    }
}

void Network::dfs(vector<int>& stack, map<int, vector<int>>& neighbours, map<int, bool>& visited, int current)
{
    // Explicit stack for DFS
    std::vector<int> dfs_stack;
    dfs_stack.push_back(current);

    while (!dfs_stack.empty())
    {
        int current = dfs_stack[dfs_stack.size() - 1];
        dfs_stack.pop_back();

        if (visited[current])
        {
            continue;
        }

        visited[current] = true;
        stack.insert(stack.begin(), current);  // Post-order

        // Push neighbors onto the stack for exploration
        if (neighbours.count(current) != 0)  // If the node has neighbors
        {
            for (auto neighbour_id : neighbours[current])
            {
                if (!visited[neighbour_id])
                {
                    dfs_stack.push_back(neighbour_id);
                }
            }
        }
    }
}

float Network::sigmoid(float val)
{
    return 1.0f / (1.0f + exp(-val));
}

//print where each link goes from and to and make sure if it's an input or output, it shows the name not it's id number
void Network::print_links()
{
    for (auto& link : links)
    {
        if (link.from_id <= 21 && link.to_id <= 21)
        {
            cout << neuron_to_name[link.from_id] << "->" << neuron_to_name[link.to_id] << " " << link.weight << " " << link.disabled << " " << link.id << endl;
        }
        else if (link.from_id <= 21)
        {
            cout << neuron_to_name[link.from_id] << "->" << link.to_id << " " << link.weight << " " << link.disabled << " " << link.id << endl;
        }
        else if (link.to_id <= 21)
        {
            cout << link.from_id << "->" << neuron_to_name[link.to_id] << " " << link.weight << " " << link.disabled << " " << link.id << endl;
        }
        else
        {
            cout << link.from_id << "->" << link.to_id << " " << link.weight << " " << link.disabled << " " << link.id << endl;
        }

    }
}

//get what button should be simulated
//if multiple have the highest output, pick output randomly
Vector2 Network::get_desire()
{
    float highest_val = -100000.0f;
    int highest_i = -1;
    vector<int> highest_is;
    //cout << "\n";
    for (int i = 0; i < 4; i++)
    {
        int id = output_ids[i];
        //cout << neurons[id].activation << " ";
        if (neurons[id].activation > highest_val)
        {
            highest_val = neurons[id].activation;
            highest_is.clear();
            highest_is.push_back(i);
        }
        else if (neurons[id].activation == highest_val)
        {
            highest_is.push_back(i);
        }
    }

    
    highest_i = highest_is[GetRandomValue(0, highest_is.size() - 1)];
    if (highest_i == 0)
    {
        return Vector2{ 0.0, -1.0 };
    }
    else if (highest_i == 1)
    {
        return Vector2{ 0.0, 1.0 };
    }
    else if (highest_i == 2)
    {
        return Vector2{ -1.0, 0.0 };
    }
    else if (highest_i == 3)
    {
        return Vector2{ 1.0, 0.0 };
    }

    return Vector2{ 0.0, 0.0 };
}

//choose a mutation using a weighted distribution
void Network::mutate(map<tuple<Mutations, int, int>, int>& mut_hist)
{
    map<Mutations, int> chances;
    //chances[Mutations::ADD_LINK] = 100;

    chances[Mutations::ADD_LINK] = 5;
    chances[Mutations::ADD_NEURON] = 3;
    chances[Mutations::EDIT_WEIGHT] = 72;
    chances[Mutations::RESET_WEIGHT] = 8;
    chances[Mutations::TOGGLE_LINK] = 3;
    chances[Mutations::NONE] = 9;

    /*chances[Mutations::ADD_LINK] = 9;
    chances[Mutations::ADD_NEURON] = 3;
    chances[Mutations::EDIT_WEIGHT] = 70;
    chances[Mutations::RESET_WEIGHT] = 8;
    chances[Mutations::TOGGLE_LINK] = 3;
    chances[Mutations::NONE] = 7;*/

    int dice = GetRandomValue(1, 100);

    for (auto &[k, v] : chances)
    {
        dice -= v;
        if (dice <= 0)
        {
            switch (k)
            {
                case Mutations::ADD_NEURON:
                    add_neuron(mut_hist);
                    break;
                case Mutations::ADD_LINK:
                    add_link(mut_hist);
                    break;
                case Mutations::EDIT_WEIGHT:
                    change_weight();
                    break;
                case Mutations::RESET_WEIGHT:
                    reset_weight();
                    break;
                case Mutations::TOGGLE_LINK:
                    toggle_link();
                    break;
                case Mutations::NONE:
                    break;
            }
        }
    }

}