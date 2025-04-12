local arr = Bytearray()
assert(#arr == 0)

for i=1,10 do
    arr[i] = 10 - i
    assert(#arr == i)
    assert(arr[i] == 10 - i)
end

for i, v in ipairs(arr) do
    assert(v == 10 - i)
end
