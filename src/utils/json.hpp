#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <fstream>
namespace lambda{
        namespace{
                using ptree = ::boost::property_tree::wptree;
        }
        class JsonReader{
                ptree pt;
        public:
                JsonReader(const std::string path){
                        std::wifstream wout (path);
                        if(!wout)
                                return;
                        try{
                                boost::property_tree::read_json(wout, pt);
                        }catch(...){
                                //TODO logging
                        }                        

                }
                ~JsonReader(){}
                template <typename T> T get(std::wstring keys, T& default_value) {
                        return pt.get(keys, default_value);
                }
                template <typename T> T get(std::wstring keys, T default_value) {
                        return pt.get(keys, default_value);
                }
                ptree getChild(std::wstring key){
                        return pt.get_child(key);
                }

        };
}