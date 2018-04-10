#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include "esel_to_pel.hpp"
#include <sdbusplus/server/object.hpp>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include "xyz/openbmc_project/Logging/Entry/server.hpp"


namespace openpower
{
namespace logging
{

namespace pel
{

using namespace phosphor::logging;

constexpr auto ADDL_DATA_PROP = "AdditionalData";
constexpr auto ESEL_HEADER = "ESEL=";
constexpr auto PEL_HEADER = "50 48 00 30";


using Properties = sdbusplus::message::variant<uint32_t,uint64_t,
                   std::string,std::vector<std::string>, bool>;
using PropertyName = std::string;
using PropertyMap = std::map<PropertyName,Properties>;

using AttributeName = std::string;
using AttributeMap = std::map<AttributeName,PropertyMap>;
using LogEntryMsg = std::pair<sdbusplus::message::object_path,AttributeMap>;


void processPels(sdbusplus::bus::bus& bus,std::string& eselStr);

void Manager::created(sdbusplus::message::message& msg )
{
    do{
        LogEntryMsg logEntry;
        msg.read(logEntry);
        std::string path(std::move(logEntry.first));

        log<level::INFO>("Read the object path",
                         entry("%s",path.c_str()));

        objPath = path;

        auto iter = logEntry.second.find("xyz.openbmc_project.Logging.Entry");

        if (iter == logEntry.second.end())
        {
            log<level::ERR>("Reached end while searching Entry");
            break;
        }

        auto attr = iter->second.find(ADDL_DATA_PROP);

        if (attr == iter->second.end())
        {
            log<level::ERR>("Reached end while searching AdditionalData");
            break;
        }

        auto& addl_data_val =
              sdbusplus::message::variant_ns::get<std::vector<std::string>>
                                                            (attr->second);

        for (auto& temp : addl_data_val )
        {
            if (strstr(temp.c_str(),ESEL_HEADER) != NULL)
            {
                log<level::INFO>("Found ESEL in the error log");
                eselStr = temp;
                processPels(bus,eselStr);

                std::ifstream phandle("parsedPelFile",std::ifstream::binary);

                if ( phandle.fail())
                {
                    log<level::ERR>("Failed to open the parsed pelfile");
                    break;
                }
                else
                {
                    phandle.seekg(0,std::ios::end);
                    uint32_t length = phandle.tellg();

                    std::string pelc;
                    pelc.resize(length + 1);

                    phandle.seekg(0,std::ios::beg);
                    phandle.read(&pelc[0],length);
                    phandle.close();

                    pelc[length+1] = '\0';

                    eselData = pelc;
                }

                pelObjects.emplace_back(std::make_unique<openpower::logging::Pel>
                                       (bus,objPath,eselData));

                break;
            }
        }
    }while(0);

    return;
}//end created


void processPels(sdbusplus::bus::bus& bus,std::string& eselStr)
{

    do {
        uint32_t length = eselStr.length();

        auto pos = eselStr.find(PEL_HEADER);

        if ( pos == std::string::npos)
        {
            log<level::ERR>("Not a valid ESEL, PEL header not present");
            break;
        }

        std::string content = eselStr;

        std::string content1;
        content1.resize(eselStr.length());

        for (uint32_t i = pos,j=0; i < length && content[i] != '\0' ; )
        {
            if ( content[i] == ' ')
            {
                i++;
            }
            else
            {
                auto temp = 0;
                for ( int k=0; k<2;k++)
                {
                    if ( content[i] >= '0' && content[i] <= '9')
                    {
                        temp = temp *16 + (content [i] - 48);
                    }
                    else if (content[i] >= 65 && content[i] <= 70) //upper case
                    {
                        temp = temp *16 + (content [i] - 55 );
                    }
                    else if(content[i] >= 97 && content[i] <= 102) //lower case
                    {
                        temp = temp *16 + (content [i] - 87);
                    }
                    else
                    {
                        content1[j] = '\0';
                    }
                    i++;
                }
                sprintf(&content1[j++],"%c",temp);
            }//end else
        }


        std::ofstream pelHandle("pelFile",std::ofstream::binary);
        pelHandle.write(content1.c_str(),length);
        pelHandle.close();

        char command [200] = {0};

        sprintf(command,"./opal-elog-parse -a -f pelFile > parsedPelFile");

        log<level::ERR>("Command used:",entry("%s",command));


        int rc = 0;
        pid_t l_pid = fork();

        if ( l_pid < 0 )
        {
            log<level::ERR>("fork failed ");
        }
        else if ( l_pid == 0 )
        {
            execl("/bin/sh","/bin/sh","-c",command,(char*)NULL);
        }
        else
        {
            int l_status = 0;
            rc = waitpid(l_pid,&l_status,0);
            if ( rc == -1)
            {
                log<level::ERR>("child process failed");
            }
        }


    }while(0);

}//end processPels


}//namespace pel
}//end namespace logging
}//end namespace openpower
