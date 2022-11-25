#include "ldap.h"

#include <utility>

// LDAP URI definieren
Ldap::Ldap() {
    this->ldapUri = "ldap://ldap.technikum-wien.at:389";
    getenv("ldapuser");
    getenv("ldappw");
    this->ldapSearchBaseDomainComponent = "dc=technikum-wien,dc=at";
    this->ldapSearchScope = LDAP_SCOPE_SUBTREE;
}
void Ldap::inputUser(string username, string password) {
    //Set username and password
    this->rawLdapUser = std::move(username);
    this->ldapBindPassword = std::move(password);
    this->ldapBindUser = "uid=" + this->rawLdapUser + ",ou=people,dc=technikum-wien,dc=at";
    string filter = "(uid="+ this->rawLdapUser+ ")";
    this->ldapSearchFilter = filter;
}
void Ldap::setupLdap() {
    int rc;
    // Connect to server
    rc = ldap_initialize(&this->ldapHandle, this->ldapUri);
    if (rc != LDAP_SUCCESS) {
        cout << "ldap failed" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "connected to LDAP server"<< this->ldapUri << endl;
    rc = ldap_set_option(this->ldapHandle, LDAP_OPT_PROTOCOL_VERSION, &this->ldapVersion);
    if (rc != LDAP_OPT_SUCCESS) {
        cout << "ldap_set_option(PROTOCOL_VERSION):"<<ldap_err2string(rc)<<endl;
        ldap_unbind_ext_s(this->ldapHandle, nullptr, nullptr);
        exit(EXIT_FAILURE);
    }
    rc = ldap_start_tls_s(this->ldapHandle, nullptr, nullptr);
    if (rc != LDAP_SUCCESS) {
        cout << "ldap_start_tls_s():"<<ldap_err2string(rc) << endl;
        ldap_unbind_ext_s(this->ldapHandle, nullptr, nullptr);
        exit(EXIT_FAILURE);
    }

}
void Ldap::ldapSearch() {
    //Search for user and show match
    int rc;
    LDAPMessage* searchResult;
    const char* ldapSearchResultAttributes[] = { "uid", "cn", nullptr };
    rc = ldap_search_ext_s(this->ldapHandle, this->ldapSearchBaseDomainComponent, this->ldapSearchScope, this->ldapSearchFilter.c_str(), (char**)ldapSearchResultAttributes, 0, nullptr, nullptr, nullptr, 500, &searchResult);
    if (rc != LDAP_SUCCESS) {
        cout << "LDAP search error:"<<ldap_err2string(rc) << endl;
        ldap_unbind_ext_s(this->ldapHandle, nullptr, nullptr);
        exit(EXIT_FAILURE);
    }
    cout << "Total results:" << ldap_count_entries(this->ldapHandle, searchResult) << endl;
    LDAPMessage* searchResultEntry;
    searchResultEntry = ldap_first_entry(this->ldapHandle, searchResult);

    // Attributes
    BerElement* ber;
    char* searchResultEntryAttribute;
    for (searchResultEntryAttribute = ldap_first_attribute(this->ldapHandle, searchResultEntry, &ber);
         searchResultEntryAttribute != nullptr;
         searchResultEntryAttribute = ldap_next_attribute(this->ldapHandle, searchResultEntry, ber))
    {
        BerValue** vals;
        if ((vals = ldap_get_values_len(this->ldapHandle, searchResultEntry, searchResultEntryAttribute)) != nullptr)
        {
            for (int i = 0; i < ldap_count_values_len(vals); i++)
            {
                cout << "\t" << searchResultEntryAttribute << ":" << vals[i]->bv_val << endl;
            }
            ldap_value_free_len(vals);
        }

        ldap_memfree(searchResultEntryAttribute);
    }
    if (ber != nullptr)
    {
        ber_free(ber, 0);
    }

    cout << endl;

    // free memory
    ldap_msgfree(searchResult);
}
bool Ldap::bindCredentials() {
    //bind credentials and check if valid
    int rc;
    BerValue bindCredentials;
    bindCredentials.bv_val=(char*)this->ldapBindPassword.c_str();
    bindCredentials.bv_len = strlen((this->ldapBindPassword).c_str());
    BerValue* servercredp;
    rc = ldap_sasl_bind_s(this->ldapHandle, this->ldapBindUser.c_str(), LDAP_SASL_SIMPLE, &bindCredentials, nullptr, nullptr, &servercredp);
    if (rc != LDAP_SUCCESS) {
        cout << "LDAP bind error:"<<ldap_err2string(rc) << endl;
        ldap_unbind_ext_s(this->ldapHandle, nullptr, nullptr);
        return false;
    }
    return true;

}

