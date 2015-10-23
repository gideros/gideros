-- simple macros created using def_ are lexically scoped
do
  def_ X 42
  assert(X == 42)
  do
    def_ X 'hello'
    assert(X == 'hello')
    do
        def_ X 999
        assert (X == 999)
    end
    assert(X == 'hello')
  end
  assert(X == 42)
end
assert (X==nil)

