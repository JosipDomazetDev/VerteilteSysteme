#pragma once
#include <ldap.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

class Ldap {
public:
    Ldap();
    void inputUser(string username, string password);
    void setupLdap();
    bool bindCredentials();
    void ldapSearch();



private:
    const char* ldapUri;
    const int ldapVersion= LDAP_VERSION3;
    string ldapBindUser;
    string rawLdapUser;
    string ldapBindPassword;
    const char* ldapSearchBaseDomainComponent;
    string ldapSearchFilter;
    ber_int_t ldapSearchScope;
    LDAP* ldapHandle{};
};
