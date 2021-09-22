#pragma once

#define SERVICE(CLASS, DOMAIN, COLLECTION)   \
                                             \
private:                                     \
    CLASS() : Service(DOMAIN, COLLECTION) {} \
                                             \
public:                                      \
    static CLASS *shared()                   \
    {                                        \
        static CLASS *single = new CLASS();  \
        return single;                       \
    }\
