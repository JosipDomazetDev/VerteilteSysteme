#pragma once
#include <ldap.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>


class Ldap {
public:
    Ldap();
    void inputUser(std::string username, std::string password);
    void setupLdap();
    bool bindCredentials();
    void ldapSearch();



private:
    const char* ldapUri;
    const int ldapVersion= LDAP_VERSION3;
    std::string ldapBindUser;
    std::string rawLdapUser;
    std::string ldapBindPassword;
    const char* ldapSearchBaseDomainComponent;
    std::string ldapSearchFilter;
    ber_int_t ldapSearchScope;
    LDAP* ldapHandle{};
};
