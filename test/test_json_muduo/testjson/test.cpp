#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

void test01()
{
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello, wat are you doing now";

    // json数据对象=》json字符串
    string sendMsg = js.dump();
    cout << sendMsg << endl;

    json obj = json::parse(sendMsg);
    cout << obj["msg_type"] << endl;
    cout << obj["from"] << endl;
    cout << obj["to"] << endl;
    cout << obj["msg"] << endl;
}

void test02()
{
    json js;
    js["id"] = {1, 2, 3, 4, 5};
    js["name"] = "zhang san";
    js["msg"]["zhangsan"] = "hello";
    js["msg"]["lisi"] = "world";
    js["obj"] = {{"obj1", "hello obj1"}, {"obj2", "hello obj2"}};
    cout << js << endl;
}

void test03()
{
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);

    json js;
    js["vec"] = vec;

    map<int, string> m;
    m.insert({1, "aaa"});
    m.insert({2, "bbb"});
    m.insert({3, "ccc"});

    js["map"] = m;
    cout << js << endl;
}

int main()
{
    test01();
    return 0;
}
