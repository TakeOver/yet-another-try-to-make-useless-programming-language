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
                        std::wifstream win (path);
                        if(!win)
                                return;
                        try{
                                boost::property_tree::read_json(win, pt);
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
                ptree& getPtree(){return pt;}

        };
        class JsonWriter{
                using ptree = ::boost::property_tree::wptree;
                ptree pt;
        public:
                JsonWriter(){}
                JsonWriter(ptree pt):pt(std::move(pt)){}
                ~JsonWriter(){}
                template <typename T> void add(std::wstring key, T value){
                        pt.add(key,value);

                }
                template <typename T> void add(std::wstring key, T& value){
                        pt.add(key,value);
                }
                void addChild(std::wstring key, ptree &pt){
                        this->pt.add_child(key, pt);
                }
                ptree & getPtree(){return pt;}
                void write2File(std::string path){
                        std::wofstream wout (path);
                        boost::property_tree::write_json(wout, pt);
                }

        };
}