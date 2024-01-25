#include "../include/simplesquirrel/object.hpp"
#include "../include/simplesquirrel/function.hpp"
#include "../include/simplesquirrel/class.hpp"
#include "../include/simplesquirrel/enum.hpp"
#include "../include/simplesquirrel/table.hpp"

#include <assert.h>
#include "../libs/squirrel/squirrel/sqvm.h"
#include "../libs/squirrel/squirrel/sqstate.h"
#include "../libs/squirrel/squirrel/sqobject.h"
#include "../libs/squirrel/squirrel/sqtable.h"

#include <squirrel.h>
#include <forward_list>

namespace ssq {
    Table::Table():Object() {
            
    }

    Table::Table(const Object& object):Object(object) {
        if (object.getType() != Type::TABLE) throw TypeException("bad cast", "TABLE", object.getTypeStr());
    }

    Table::Table(HSQUIRRELVM vm):Object(vm) {
        sq_newtable(vm);
        sq_getstackobj(vm, -1, &obj);
        sq_addref(vm, &obj);
        sq_pop(vm,1); // Pop table
    }

    Table::Table(const Table& other):Object(other) {
            
    }

    Table::Table(Table&& other) NOEXCEPT :Object(std::forward<Table>(other)) {
            
    }

    std::vector<Object> Table::getKeys() const
    {
      std::vector<Object> keys;

      SQTable * tb = _table(obj);

      SQInteger ridx = 0;
      SQObjectPtr key, val;
      while( (ridx = tb->Next(true, ridx, key, val)) != -1 )
      {
        vm->Push(key);
        keys.push_back(detail::pop<Object>(vm, -1));
      }

      return keys;
    }

    TableMap Table::getMap() const
    {
      TableMap ret;

      SQTable * tb = _table(obj);

      SQInteger ridx = 0;
      SQObjectPtr key, val;
      while( (ridx = tb->Next(true, ridx, key, val)) != -1 )
      {
        vm->Push(key);
        Object okey = detail::pop<Object>(vm, -1);
        vm->Push(val);
        Object oval = detail::pop<Object>(vm, -1);

        switch( okey.getType() )
        {
          case Type::INTEGER:
          {
            SQInteger i = okey.toInt();
            ret.try_emplace(i, oval);
          }
          break;

          case Type::STRING:
          {
            sqstring s = okey.toString();
            ret.try_emplace(s, oval);
          }
          break;
        }
      }

      return ret;
    }

    Function Table::findFunc(const SQChar* name) const {
        Object object = Object::find(name);
        return Function(object);
    }

    Class Table::findClass(const SQChar* name) const {
        Object object = Object::find(name);
        return Class(object);
    }

    Enum Table::addEnumGlobal(const SQChar* name) {
        Enum enm(vm);
        sq_pushconsttable(vm);
        sq_pushstring(vm, name, scstrlen(name));
        detail::push<Object>(vm, enm);
        sq_newslot(vm, -3, false);
        sq_pop(vm,1); // pop table
        return std::move(enm);
    }

    Enum Table::addEnumLocal(const SQChar* name) {
        Enum enm(vm);
        //sq_pushconsttable(vm);
        sq_pushobject(vm, obj);
        sq_pushstring(vm, name, scstrlen(name));
        detail::push<Object>(vm, enm);
        sq_newslot(vm, -3, false);
        sq_pop(vm,1); // pop table
        return std::move(enm);
    }

    Table Table::addTable(const SQChar* name) {
        Table table(vm);
        sq_pushobject(vm, obj);
        sq_pushstring(vm, name, scstrlen(name));
        detail::push<Object>(vm, table);
        sq_newslot(vm, -3, false);
        sq_pop(vm,1); // pop table
        return std::move(table);
    }

    size_t Table::size() {
        sq_pushobject(vm, obj);
        SQInteger s = sq_getsize(vm, -1);
        sq_pop(vm, 1);
        return static_cast<size_t>(s);
    }

    Table& Table::operator = (const Table& other){
        Object::operator = (other);
        return *this;
    }

    Table& Table::operator = (Table&& other) NOEXCEPT {
        Object::operator = (std::forward<Table>(other));
        return *this;
    }
}
