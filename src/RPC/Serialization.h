#pragma once

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <jackson/FileReadStream.h>
#include <jackson/Document.h>
#include <jackson/Writer.h>
#include <jackson/FileWriteStream.h>
#include <jackson/Value.h>

json::Value readJsonFromFile(char* filePath);
void parseRpc(json::Value& rpc);
void parseProto(json::Value& proto);