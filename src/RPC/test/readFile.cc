#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <jackson/FileReadStream.h>
#include <jackson/Document.h>
#include <jackson/Writer.h>
#include <jackson/FileWriteStream.h>
using namespace std;

int main()
{
    FILE* input = fopen("./document.json","r");
    assert(input != nullptr);
    json::FileReadStream is(input);
    json::Document proto;
    auto err = proto.parseStream(is);
    json::FileWriteStream os(stdout);
    json::Writer writer(os);
    proto.writeTo(writer);
    fclose(input);
    return 0;
}
