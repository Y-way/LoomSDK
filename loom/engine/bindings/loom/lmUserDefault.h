/*
* ===========================================================================
* Loom SDK
* Copyright 2011, 2012, 2013
* The Game Engine Company, LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* ===========================================================================
*/

#include "loom/script/loomscript.h"

class UserDefault
{
    static UserDefault shared;
    
public:
    bool getBoolForKey(const char *k, bool v);
    int getIntegerForKey(const char *k, int v);
    float getFloatForKey(const char *k, float v);
    utString getStringForKey(const char *k, const char* v);
    double getDoubleForKey(const char *k, double v);
    
    void setBoolForKey(const char *k, bool v);
    void setIntegerForKey(const char *k, int v);
    void setFloatForKey(const char *k, float v);
    void setStringForKey(const char *k, const char * v);
    void setDoubleForKey(const char *k, double v);
    
    bool purge();
    
    static bool purgeSharedUserDefault()
    {
        return shared.purge();
    }
    
    static UserDefault *sharedUserDefault()
    {
        return &shared;
    }
};
