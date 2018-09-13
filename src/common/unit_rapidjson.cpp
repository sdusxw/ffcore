//
// Created by boon on 17-11-14.
//

//#include "rapidjson/document.h"
//#include "rapidjson/writer.h"
//#include "rapidjson/stringbuffer.h"

#include "rapidjson_lib.h"

#include "utils.h"

#include <stdio.h>
#include <unistd.h>

using namespace rapidjson;

void write_json();
void read_json();

bool get_json(std::string& _json);

bool get_json2(std::string& _json);

int main(int argc, char** argv)
{
    write_json();

    return 0;
}

void write_json()
{
    std::string str;

    int i = 0;
    while(1){

        get_json2(str);

        printf("[%s] %d, %s\n", printTime().c_str(), i++, str.c_str());

        if(i >= 65535)
            i = 0;

        usleep(1);
    }

}

bool get_json(std::string& _json)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);

    writer.StartObject();               // Between StartObject()/EndObject(),
    writer.Key("hello");                // output a key,
    writer.String("world");             // follow by a value.

    writer.Key("t");
    writer.Bool(true);

    writer.Key("f");
    writer.Bool(false);

    writer.Key("n");
    writer.Null();

    writer.Key("i");
    writer.Uint(123);

    writer.Key("pi");
    writer.Double(3.1416);

    writer.Key("a");
    writer.StartArray();                // Between StartArray()/EndArray(),
    for (unsigned i = 0; i < 4; i++)
        writer.Uint(i);                 // all values are elements of the array.
    writer.EndArray();
    writer.EndObject();

    // {"hello":"world","t":true,"f":false,"n":null,"i":123,"pi":3.1416,"a":[0,1,2,3]}
    //cout << s.GetString() << endl;

    //printf("%s json[%s]\n", __FUNCTION__, s.GetString());

    _json = s.GetString();
}

bool get_json2(std::string& _json)
{

    char device_name_buf[64] = { "device_name" };
    char device_location_buf[128] = {"device_location"};
    char device_lable_buf[128] = { "device_label" };
    char point_name_buf[64] = { "point_name" };

    rapidjson::Document doc;
    doc.SetObject();

    using rapidjson::Value;

    // convert dom to string.
    Value ValueObject(rapidjson::kObjectType);

    ValueObject.SetString(StringRef(device_name_buf));
    doc.AddMember("name", ValueObject, doc.GetAllocator());

    ValueObject.SetInt(12);
    doc.AddMember("id", ValueObject, doc.GetAllocator());

    ValueObject.SetString(StringRef(device_location_buf));
    doc.AddMember("location", ValueObject, doc.GetAllocator());

    ValueObject.SetString(StringRef(device_lable_buf));
    doc.AddMember("label", ValueObject, doc.GetAllocator());

    doc.AddMember("groupno", "1", doc.GetAllocator());

    Value item(rapidjson::kArrayType);

    //Ã¿¸öÉè±¸¶àÉÙµã
    for (int j = 1; j <= 5; j++)
    {
        Value item1(rapidjson::kObjectType);

        ValueObject.SetString(StringRef(point_name_buf));
        item1.AddMember("tag", ValueObject, doc.GetAllocator());

        if (j % 2 == 0)
        {
            ValueObject.SetInt(j % 2);
            item1.AddMember("offset", ValueObject, doc.GetAllocator());
            ValueObject.SetInt(j % 2);
            item1.AddMember("type", ValueObject, doc.GetAllocator());
        }
        else
        {
            ValueObject.SetInt(j % 2);
            item1.AddMember("offset", ValueObject, doc.GetAllocator());
            ValueObject.SetInt(j % 2);
            item1.AddMember("type", ValueObject, doc.GetAllocator());
        }

        ValueObject.SetDouble(12.34);
        item1.AddMember("value", ValueObject, doc.GetAllocator());

        item.PushBack(item1, doc.GetAllocator());


    }

    doc.AddMember("point", item, doc.GetAllocator());

    doc.AddMember("timestamp", "14823434324", doc.GetAllocator());
    doc.AddMember("quality", "000000080", doc.GetAllocator());

    rapidjson::StringBuffer buffer;//in rapidjson/stringbuffer.h
    rapidjson::Writer<StringBuffer> writer(buffer); //in rapidjson/writer.h
    doc.Accept(writer);
    //str_out = buffer.GetString();
    _json = buffer.GetString();
}

void read_json()
{
    char buf[]={"{\n"
                        "    \"hello\": \"rapidjson\",\n"
                        "    \"t\": true,\n"
                        "    \"f\": false,\n"
                        "    \"n\": null,\n"
                        "    \"i\": 2432902008176640000,\n"
                        "    \"pi\": 3.1416,\n"
                        "    \"a\": [\n"
                        "        1,\n"
                        "        2,\n"
                        "        3,\n"
                        "        4,\n"
                        "        5,\n"
                        "        6,\n"
                        "        7,\n"
                        "        8,\n"
                        "        9,\n"
                        "        10,\n"
                        "        \"Lua\",\n"
                        "        \"Mio\"\n"
                        "    ],\n"
                        "    \"author\": \"Milo Yip\"\n"
                        "}"};
}