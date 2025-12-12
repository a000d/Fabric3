#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <assert.h>


using namespace std;

//          update: 20240811
//          #include "String_Utils.hpp"



namespace String_Utils {
#define _S(c) std::to_string(c)

struct Divide_Info {
    size_t pos;
    size_t len;

    Divide_Info(const size_t pos, const size_t len) {
        this->len = len;
        this->pos = pos;
    }
};

struct Class_to_Str {

    string str;
    Class_to_Str(int v) {
        str = std::to_string(v);
    }
    Class_to_Str(size_t v) {
        str = std::to_string(v);
    }
    Class_to_Str(unsigned int v) {
        str = std::to_string(v);
    }
    Class_to_Str(string v) {
        str = v;
    }
    Class_to_Str(float v) {
        str = std::to_string(v);
    }
    Class_to_Str(const char* v) {
        str = v;
    }
};


static vector<size_t> Split_Id(string str, string sp) {

    vector<size_t> id_list;
    size_t start = 0;

    while (true) {
        size_t id = (size_t)str.find(sp, start);
        if (id == SIZE_MAX) {
            break;
        }
        id_list.push_back(id);
        start = id + (size_t)sp.size();
    }
    return id_list;
}

static vector<size_t> Split_Id_Exclude_Sign(string str, string sp,string exclude_sign) {

    vector<size_t> id_list = Split_Id(str, sp);
    vector<size_t> ex_list = Split_Id(str, exclude_sign);

    vector<size_t> id_list_result;

    // assert(ex_list.size()%2==0,"exclude symbol err");
    size_t range_count = ex_list.size() / 2;

    size_t range_check_start = 0;

    for (size_t i = 0; i < id_list.size();i++) {

        bool excluded = false;
        for (size_t e = range_check_start; e < range_count;e++) {
            if (id_list[i]>ex_list[2*e]) {
                if (id_list[i]<ex_list[2*e + 1]) {
                    excluded = true;
                    break;
                }
                else {
                    range_check_start++;
                }
            }
            else {
                break;
            }
        }
        if (!excluded) {
            id_list_result.push_back(id_list[i]);
        }

    }
    return id_list_result;
}

static vector<string> Split(string str, vector<string> sp_list) {

    vector<Divide_Info> list;

    for (string& sp : sp_list) {

        vector<size_t> res = Split_Id(str, sp);
        size_t len = (size_t)sp.size();

        for (size_t p : res) {
            list.push_back({ p,len });
        }
    }
    auto sort_fun = [](const Divide_Info& v1, const Divide_Info& v2) {
        return v1.pos < v2.pos;

    };


    sort(list.begin(), list.end(), sort_fun);

    vector<string> res_list;

    size_t chk = 0;

    for (size_t i = 0; i < list.size(); i++) {

        string sub = str.substr(chk, list[i].pos - chk);
        if (sub != "") {
            res_list.push_back(sub);
        }
        chk = list[i].pos + list[i].len;
    }
    string sub = str.substr(chk, str.size() - chk);
    if (sub != "") {
        res_list.push_back(sub);
    }

    return res_list;
}

static vector<string> Split(string str, vector<string> sp_list, bool include_space,string exculde_sign) {

    vector<Divide_Info> list;

    for (string& sp: sp_list) {
        vector<size_t> res;

        if (exculde_sign == "") {
            res =  Split_Id(str,sp);
        }
        else {
            res =  Split_Id_Exclude_Sign(str,sp,exculde_sign);
        }

        size_t len = (size_t)sp.size();

        for (size_t p: res) {
            list.push_back({p,len});
        }
    }
    auto sort_fun = [](const Divide_Info& v1,const Divide_Info& v2) {
        return v1.pos < v2.pos;

    };


    sort(list.begin(),list.end(), sort_fun);

    vector<string> res_list;

    size_t chk = 0;

    for (size_t i = 0; i < list.size();i++) {

        string sub = str.substr(chk, list[i].pos - chk);
        if (sub!=""||include_space) {
            res_list.push_back(sub);
        }
        chk = list[i].pos + list[i].len;
    }
    string sub = str.substr(chk, str.size()-chk);
    if (sub!=""||include_space) {
        res_list.push_back(sub);
    }

    return res_list;
}
static string Replace(const string str,const string from,const string to)
{
    string res = str;

    vector<size_t> list = Split_Id(res, from);
    size_t str_from_len = (size_t)from.size();


    for (size_t i = (size_t)list.size() - 1; i != SIZE_MAX;i--)
    {
        res.replace(list[i],str_from_len ,to);
    }

    return res;

}

static string _fmt(const string str,const vector<string> str_list) {

    string res = str;
    vector<size_t> list = Split_Id(res, "{}");
    size_t times = (size_t)min(list.size(), list.size());

    for (size_t i = times - 1; i != SIZE_MAX; i--) {

        res.replace(list[i], 2, str_list[i]);

    }
    return res;
}
static string fmt(const string str, const vector<Class_to_Str> c_list) {
    vector<string> str_list;

    for (const Class_to_Str& v: c_list) {
        str_list.push_back(v.str);
    }

    return _fmt(str, str_list);
}

string Read_All(const string path) {

    if (!filesystem::directory_entry(path).exists()) {
        //cout << "file not found" << endl;
        return "";
    }
    size_t size = filesystem::directory_entry(path).file_size();

    char* buf = new char[size + 4];
    memset(buf,0,size + 4);

    ifstream fin(path,ios::in|ios::binary);

    fin.read(buf,size);

    string res = buf;

    fin.close();

    delete[](buf);

    return res;
}
void Write_All(const string path,const string content) {

    ofstream fout(path,ios::out|ios::binary);
    fout << content;

    fout.close();
}


void Write_Line(const string path,const string content) {

    ofstream fout(path,ios::app|ios::binary);
    fout << content << endl;
    fout.close();

}

bool Has_Str_or(string str,const vector<string> sub_list) {

    for (const string& sub : sub_list) {

        size_t pos = str.find(sub);
        if (pos != SIZE_MAX) {

            return true;
        }
    }
    return false;
}
bool Has_Str_and(string str,const vector<string> sub_list) {

    for (const string& sub : sub_list) {

        size_t pos = str.find(sub);
        if (pos == SIZE_MAX) {

            return false;
        }
    }
    return true;
}

class CSV {

    struct Line {
        vector<string> line;

        string& operator[](int x) {
            if (line.size()<=x) {
                int size_lack = x - (int)line.size() + 1;

                for (int i = 0; i < size_lack;i++) {
                    line.push_back("");
                }
            }
            return line[x];
        }
        Line(){}
        int size() {
            return (int)line.size();
        }
        void push_back(const string val) {
            line.push_back(val);
        }
        Line(vector<string> line) {
            this->line = line;
        }
    };

private:
    string path;
    string content;
    uintmax_t file_size;

    vector<Line> table;


private:

public:
    CSV() {

    }
    CSV(const string path) {
        this->path = path;

        file_size = filesystem::directory_entry(path).file_size();

        char* buf = new char[file_size+64];
        memset(buf, 0, file_size + 64);

        ifstream fin(path,ios::in|ios::binary);
        fin.read(buf,file_size);
        fin.close();

        if (buf[0]==0xef &&buf[1]==0xbb &&buf[2]==0xbf) {
            content = buf+3;
        }
        else {
            content = buf;
        }
        Init_By_CSV_Content(content);

        delete[](buf);

    }

    void Init_By_CSV_Content(const string csv_str) {
        content = csv_str;

        content = Replace(content,"\r","");

        vector<string> line_list = Split(content, { "\n" }, true,"");
        for (int i = 0; i < (int)line_list.size();i++) {

            if (i==line_list.size()-1) {//���һ��
                if (line_list[i]=="") {//���һ��Ϊ��
                    break;
                }
            }

            vector<string> line_split = Split(line_list[i], { "," }, true,"\"");

            for (size_t i = 0; i < line_split.size();i++) {
                string& str = line_split[i];
                if (str[0]=='\"') {
                    str = str.substr(1,str.size()-2);
                }
                str = Replace(str,"\"\"","\"");
            }



            table.push_back({ line_split });
        }
    }

    string Get(const int y,const int x) {
        if (table.size()<=y) {
            int size_lack = y - table.size() + 1;

            for (int i = 0; i < size_lack;i++) {
                table.push_back(Line());
            }
        }
        if (table[y].size()<=x) {
            int size_lack = x - table[y].size() + 1;

            for (int i = 0; i < size_lack;i++) {
                table[y].push_back("");
            }
        }
        return table[y][x];
    }
    void Set(const int y,const int x,const string content) {

        if (table.size()<=y) {
            int size_lack = y - table.size() + 1;

            for (int i = 0; i < size_lack;i++) {
                table.push_back(Line());
            }
        }
        if (table[y].size()<=x) {
            int size_lack = x - (int)table[y].size() + 1;

            for (int i = 0; i < size_lack;i++) {
                table[y].push_back("");
            }
        }
        table[y][x] = content;
    }
    Line& operator[] (int y) {
        if (table.size()<=y) {
            int size_lack = y - table.size() + 1;

            for (int i = 0; i < size_lack;i++) {
                table.push_back(Line());
            }
        }

        return table[y];
    }
    int height() {
        return table.size();
    }
    int Max_width() {
        int max_tmp = 0;

        for (auto l:table) {
            max_tmp = max(max_tmp,l.size());
        }
        return max_tmp;
    }
    string Encode() {
        string encode_str = "";

        for (int h = 0; h < (int)table.size();h++) {
            for (int w = 0; w < (int)table[h].size();w++) {
                string value = table[h][w];
                bool special_code = false;
                if (Has_Str_or(value, {",","\""})) {//������������������Ű���
                    special_code = true;
                }
                if (special_code) {
                    encode_str += "\"";
                    value = Replace(value,"\"","\"\"");
                }
                encode_str += value;


                if (special_code) {
                    encode_str += "\"";
                }

                if (w!=(int)table[h].size()-1) {
                    encode_str += ",";//���Ӷ���
                }

            }
            encode_str += "\n";

        }

        content = encode_str;
        return encode_str;
    }

    void Save(const string path) {
        Write_All(path,content);
    }
    void Save() {
        Write_All(this->path,content);
    }
    ~CSV() {

    }


};
};

