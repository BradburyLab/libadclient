/*
  Python wrapper around C++ class.
*/
#include <Python.h>
#include "adclient.h"

static PyObject *ADBindError;
static PyObject *ADSearchError;
static PyObject *ADOperationalError;
static int error_num;

PyObject *vector2list(vector <string> vec) {
       string st;
       PyObject *list = PyList_New(vec.size());;

       for (unsigned int j=0; (j < vec.size()); j++) {
           if (PyList_SET_ITEM(list, j, PyString_FromString(vec[j].c_str())) < 0)
              return NULL;
       }
       return list;
}

adclient *convert_ad(PyObject *obj) {
       void * temp = PyCObject_AsVoidPtr(obj);
       return static_cast<adclient*>(temp);
}

static PyObject *wrapper_get_error_num(PyObject *self, PyObject *args) {
       return Py_BuildValue("i", error_num);
}

static PyObject *wrapper_new_adclient(PyObject *self, PyObject *args) {
       error_num = 0;
       adclient *obj = new adclient();
       return PyCObject_FromVoidPtr(obj, NULL);
}

static PyObject *wrapper_login_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *uri, *binddn, *bindpw, *search_base;

       if (!PyArg_ParseTuple(args, "Ossss", &obj, &uri, &binddn, &bindpw, &search_base)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->login(uri, binddn, bindpw, search_base);
       }
       catch(ADBindException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADBindError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_search_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *ou, *filter;
       PyObject * listObj;
       PyObject * strObj;
       unsigned int numLines;
       int scope;

       map < string, map < string, vector<string> > > res;

       if (!PyArg_ParseTuple(args, "OsisO!", &obj, &ou, &scope, &filter, &PyList_Type, &listObj)) return NULL;

       if ((numLines = PyList_Size(listObj)) < 0) return NULL; /* Not a list */

       vector <string> attrs;

       for (unsigned int i=0; i<numLines; i++) {
          strObj = PyList_GetItem(listObj, i);
          string item = PyString_AsString(strObj);
          attrs.push_back(item);
       }

       adclient *ad = convert_ad(obj);
       try {
          res = ad->search(ou, scope, filter, attrs);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }

       PyObject *res_dict = PyDict_New();
       map < string, map < string, vector<string> > >::iterator res_it;
       for ( res_it=res.begin() ; res_it != res.end(); res_it++ ) {
           PyObject *attrs_dict = PyDict_New();
           string dn = (*res_it).first;
           map < string, vector<string> > attrs = (*res_it).second;
           map < string, vector<string> >::iterator attrs_it;
           for ( attrs_it=attrs.begin() ; attrs_it != attrs.end(); attrs_it++ ) {
               string attribute = (*attrs_it).first;
               vector<string> values_v = (*attrs_it).second;
               PyObject *values_list = vector2list(values_v);
               if (PyDict_SetItemString(attrs_dict, attribute.c_str(), values_list) < 0) return NULL;
           }
           if (PyDict_SetItemString(res_dict, dn.c_str(), attrs_dict) < 0) return NULL;
       }
       return res_dict;
}

static PyObject *wrapper_searchDN_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *filter;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &filter)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->searchDN(filter);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getUserGroups_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getUserGroups(user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getUsersInGroup_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *group;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &group)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getUsersInGroup(group);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_groupAddUser_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *group, *user;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &group, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->groupAddUser(group, user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_groupRemoveUser_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *group, *user;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &group, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->groupRemoveUser(group, user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_ifDialinUser_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          return Py_BuildValue("i", ad->ifDialinUser(user));
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
}

static PyObject *wrapper_getDialinUsers_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getDialinUsers();
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getObjectDN_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       string result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getObjectDN(user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return Py_BuildValue("s", result.c_str());
}

static PyObject *wrapper_ifUserDisabled_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          return Py_BuildValue("i", ad->ifUserDisabled(user));
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
}

static PyObject *wrapper_ifDNExists_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *dn;
       char *objectclass;

       if (!PyArg_ParseTuple(args, "Oss", &obj, &dn, &objectclass)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          return Py_BuildValue("i", ad->ifDNExists(dn, objectclass));
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str()); 
            return NULL; 
       }
}

static PyObject *wrapper_getAllOUs_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       vector <string> result;

       if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getAllOUs();
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getOUsInOU_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *OU;
       vector <string> result;

       if (!PyArg_ParseTuple(args, "Os", &obj, &OU)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getOUsInOU(OU);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getUsersInOU_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *OU;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &OU)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getUsersInOU(OU);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getUsersInOU_SubTree_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *OU;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &OU)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getUsersInOU_SubTree(OU);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getGroups_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getGroups();
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getUsers_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getUsers();
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getUserDisplayName_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user_short;
       string result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user_short)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getUserDisplayName(user_short);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return Py_BuildValue("s", result.c_str());
}

static PyObject *wrapper_getObjectAttribute_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user_short, *attribute;
       vector <string> result;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user_short, &attribute)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getObjectAttribute(user_short, attribute);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       return vector2list(result);
}

static PyObject *wrapper_getObjectAttributes_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *object_short;
       map <string, vector <string> > result;
       if (!PyArg_ParseTuple(args, "Os", &obj, &object_short)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          result = ad->getObjectAttributes(object_short);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }

       string attr;
       vector <string> values;

       PyObject *fin_list = PyList_New(result.size());
       PyObject *tuple;
       PyObject *temp_list;

       map<string, vector<string> >::iterator it;
       for (it = result.begin(); it != result.end(); ++it) {
           attr = it->first;
           values = it->second;
           temp_list = PyList_New(values.size());
           for (unsigned int j = 0; j < values.size(); ++j) {
               PyList_SET_ITEM(temp_list, j, PyString_FromString(values[j].c_str()));
           }
           tuple = Py_BuildValue("(s,N)", attr.c_str(), temp_list);
           PyList_SET_ITEM(fin_list, std::distance(result.begin(), it), tuple);
       }
       return fin_list;
}

static PyObject *wrapper_CreateUser_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *cn, *short_name, *container;
       if (!PyArg_ParseTuple(args, "Osss", &obj, &cn, &container, &short_name)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->CreateUser(cn, container, short_name);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_DeleteDN_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *dn;
       if (!PyArg_ParseTuple(args, "Os", &obj, &dn)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->DeleteDN(dn);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_CreateOU_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *ou;
       if (!PyArg_ParseTuple(args, "Os", &obj, &ou)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->CreateOU(ou);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str()); 
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_UnLockUser_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->UnLockUser(user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserDescription_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *dn, *descr;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &dn, &descr)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserDescription(dn, descr);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserPassword_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *password;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &password)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserPassword(user, password);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserDialinAllowed_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserDialinAllowed(user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserDialinDisabled_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserDialinDisabled(user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserSN_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *sn;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &sn)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserSN(user, sn);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserInitials_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *initials;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &initials)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserInitials(user, initials);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str()); 
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserGivenName_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *givenName;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &givenName)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserGivenName(user, givenName);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());  
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserDisplayName_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *displayName;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &displayName)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserDisplayName(user, displayName);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());  
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserRoomNumber_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *roomNum;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &roomNum)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserRoomNumber(user, roomNum);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());  
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       } 
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserAddress_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *streetAddress;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &streetAddress)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserAddress(user, streetAddress);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());  
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserInfo_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *info;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &info)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserInfo(user, info);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       } 
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserTitle_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *title;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &title)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->setUserTitle(user, title);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL; 
       } 
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserDepartment_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *department;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &department)) return NULL;
       adclient *ad = convert_ad(obj);
       try { 
          ad->setUserDepartment(user, department);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL; 
       } 
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserCompany_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *company;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &company)) return NULL;
       adclient *ad = convert_ad(obj);
       try { 
          ad->setUserCompany(user, company);
       }
       catch(ADSearchException& ex) { 
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL; 
       } 
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject *wrapper_setUserPhone_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user, *phone;
       if (!PyArg_ParseTuple(args, "Oss", &obj, &user, &phone)) return NULL;
       adclient *ad = convert_ad(obj);
       try { 
          ad->setUserPhone(user, phone);
       }
       catch(ADSearchException& ex) { 
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL; 
       } 
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       } 
       Py_INCREF(Py_None);
       return Py_None;
}

static PyObject * wrapper_UnlockUser_adclient(PyObject *self, PyObject *args) {
       PyObject *obj;
       char *user;
       if (!PyArg_ParseTuple(args, "Os", &obj, &user)) return NULL;
       adclient *ad = convert_ad(obj);
       try {
          ad->UnlockUser(user);
       }
       catch(ADSearchException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADSearchError, ex.msg.c_str());
            return NULL;
       }
       catch(ADOperationalException& ex) {
            error_num = ex.code;
            PyErr_SetString(ADOperationalError, ex.msg.c_str());
            return NULL;
       }
       Py_INCREF(Py_None);
       return Py_None;
}

static PyMethodDef adclient_methods[] = {
       { "new_adclient", wrapper_new_adclient, 1 },
       { "login_adclient", wrapper_login_adclient, 1 },
       { "searchDN_adclient", wrapper_searchDN_adclient, 1},
       { "search_adclient", wrapper_search_adclient, 1},
       { "getUserGroups_adclient", wrapper_getUserGroups_adclient, 1 },
       { "getUsersInGroup_adclient", wrapper_getUsersInGroup_adclient, 1},
       { "groupAddUser_adclient", wrapper_groupAddUser_adclient, 1 },
       { "groupRemoveUser_adclient", wrapper_groupRemoveUser_adclient, 1 },
       { "ifDialinUser_adclient", wrapper_ifDialinUser_adclient, 1 },
       { "getDialinUsers_adclient", wrapper_getDialinUsers_adclient, 1 },
       { "getObjectDN_adclient", wrapper_getObjectDN_adclient, 1 },
       { "ifUserDisabled_adclient", wrapper_ifUserDisabled_adclient, 1 },
       { "getAllOUs_adclient", wrapper_getAllOUs_adclient, 1 },
       { "getOUsInOU_adclient", wrapper_getOUsInOU_adclient, 1},
       { "getUsersInOU_adclient", wrapper_getUsersInOU_adclient, 1 },
       { "getUsersInOU_SubTree_adclient", wrapper_getUsersInOU_SubTree_adclient, 1},
       { "getGroups_adclient", wrapper_getGroups_adclient, 1 },
       { "getUsers_adclient", wrapper_getUsers_adclient, 1 },
       { "getUserDisplayName_adclient", wrapper_getUserDisplayName_adclient, 1 },
       { "getObjectAttribute_adclient", wrapper_getObjectAttribute_adclient, 1 },
       { "getObjectAttributes_adclient", wrapper_getObjectAttributes_adclient, 1 },
       { "CreateUser_adclient", wrapper_CreateUser_adclient, 1 },
       { "DeleteDN_adclient", wrapper_DeleteDN_adclient, 1 },
       { "CreateOU_adclient", wrapper_CreateOU_adclient, 1 },
       { "UnLockUser_adclient", wrapper_UnLockUser_adclient, 1 },
       { "setUserDescription_adclient", wrapper_setUserDescription_adclient, 1 },
       { "setUserPassword_adclient", wrapper_setUserPassword_adclient, 1 },
       { "setUserDialinAllowed_adclient", wrapper_setUserDialinAllowed_adclient, 1 },
       { "setUserDialinDisabled_adclient", wrapper_setUserDialinDisabled_adclient, 1 },
       { "setUserSN_adclient", wrapper_setUserSN_adclient, 1 },
       { "setUserInitials_adclient", wrapper_setUserInitials_adclient, 1 },
       { "setUserGivenName_adclient", wrapper_setUserGivenName_adclient, 1 },
       { "setUserDisplayName_adclient", wrapper_setUserDisplayName_adclient, 1 },
       { "setUserRoomNumber_adclient", wrapper_setUserRoomNumber_adclient, 1 },
       { "setUserAddress_adclient", wrapper_setUserAddress_adclient, 1 },
       { "setUserInfo_adclient", wrapper_setUserInfo_adclient, 1 },
       { "setUserTitle_adclient", wrapper_setUserTitle_adclient, 1 },
       { "setUserDepartment_adclient", wrapper_setUserDepartment_adclient, 1 },
       { "setUserCompany_adclient", wrapper_setUserCompany_adclient, 1 },
       { "setUserPhone_adclient", wrapper_setUserPhone_adclient, 1 },
       { "UnlockUser_adclient", wrapper_UnlockUser_adclient, 1 },
       { "ifDNExists_adclient", wrapper_ifDNExists_adclient, 1 },
       { "get_error_num", wrapper_get_error_num, 1 },
       { NULL, NULL }
};

extern "C" void init_adclient() {
       PyObject *m;
       m = Py_InitModule("_adclient", adclient_methods);
       ADBindError = PyErr_NewException("ADBindError.error", NULL, NULL);
       ADSearchError = PyErr_NewException("ADSearchError.error", NULL, NULL);
       ADOperationalError = PyErr_NewException("ADOperationalError.error", NULL, NULL);
       Py_INCREF(ADBindError);
       Py_INCREF(ADSearchError);
       Py_INCREF(ADOperationalError);
       PyModule_AddObject(m, "ADBindError", ADBindError);
       PyModule_AddObject(m, "ADSearchError", ADSearchError);
       PyModule_AddObject(m, "ADOperationalError", ADOperationalError);
}
