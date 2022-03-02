
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>

struct Param
{
    int value;
    int lower_bound;
    int upper_bound;
};

void ParseArgs(int argc, char* argv[], std::map<std::string,Param>& param_defaults, int hull_max )
{
    std::vector<int> object_count_defaults 
    { 1000, hull_max, 2000, 10000, 10000000, 5000000 };

    for(int i = 0; i < argc; i++)
    {
        std::string arg = argv[i];
        auto it = param_defaults.find(arg);
        if( it != param_defaults.end() )
        {
            // check if no. of args is correct
            assert( i + 1 < argc && "Missing command line arg");

            // checks if arg is pos integer
            std::string arg_value = argv[i+1];
            assert( std::all_of(arg_value.begin(),arg_value.end(),::isdigit) && "Command line arg must be a positive integer" );

            const std::string& param_name = (*it).first;
            const Param& par = param_defaults[param_name];
            int value = std::stoi( arg_value );
            
            // checks if arg is in the valid range
            assert( value >= par.lower_bound && value <= par.upper_bound && &("Command line arg not in range: " + param_name ) );
            param_defaults[param_name].value = std::stoi(arg_value);
        }
    }
    // if object count not supplied by argument then
    if( param_defaults["-n"].value == 0 )
    {
        // object count is a special case as it depends on both the algorithm and whether the program is being benchmarked
        // the first 3 elements are the defaults for the visual program the last three are the defaults for the benchmarks
        param_defaults["-n"].value = object_count_defaults[(param_defaults["-algo"].value-1)+
                                                            3*param_defaults["-benchmark"].value];
    }

    // hack to avoid print statements on profiled runs
    if( param_defaults["-out"].value == 0 )
        std::cout.setstate(std::ios_base::badbit);
}