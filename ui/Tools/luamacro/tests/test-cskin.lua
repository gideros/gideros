-- run like so: luam -lcskin test-cskin.lua

class Named {
  def _init(self,name) { -- name for ctor
    self.name = name
  }

  def __tostring(self) {  -- metamethod
    return self.name
  }
}

class Shamed: Named { -- doesn't have to define a ctor
  def __tostring(self) {
    return "shame on "..self.name
  }
}

class Person : Named {
  def _init(self,name,age) { -- ctor must call inherited ctor explicitly
    Named._init(self,name)
    self:set_age(age)
  }

  def set_age(self,age) { -- plain method
    self.age = age;
  }

  def __tostring(self) {
    return Named.__tostring(self)..' age '..self.age
  }
}

a = Named 'Alice'
print(a)
b = Shamed 'Job'
print(b)

aa = {|Named(n)| n in {'Johan','Peter','Mary'}}

forall(a in aa) { print(a) }

p = Person ('Bob',12)
print(p)



