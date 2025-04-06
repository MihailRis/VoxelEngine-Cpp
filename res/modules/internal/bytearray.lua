local MIN_CAPACITY = 8
local _type = type
local FFI = ffi

FFI.cdef[[
    void* malloc(size_t);
    void free(void*);
    typedef struct {
        unsigned char* bytes;
        int size;
        int capacity;
    } bytearray_t;
]]

local malloc = FFI.C.malloc
local free = FFI.C.free

local function grow_buffer(self, elems)
    local new_capacity = math.ceil(self.capacity / 0.75 + elems)
    local prev = self.bytes
    self.bytes = malloc(new_capacity)
    FFI.copy(self.bytes, prev, self.size)
    self.capacity = new_capacity
    free(prev)
end

local function count_elements(b)
    local elems = 1
    if _type(b) ~= "number" then
        elems = #b
    end
    return elems
end

local bytearray_methods = {
    append=function(self, b)
        local elems = count_elements(b)
        if self.size + elems > self.capacity then
            grow_buffer(self, elems)
        end
        if _type(b) == "number" then
            self.bytes[self.size] = b
        else
            for i=1, #b do
                self.bytes[self.size + i - 1] = b[i]
            end
        end
        self.size = self.size + elems
    end,
    insert=function(self, index, b)
        local elems = count_elements(b)
        if self.size + elems >= self.capacity then
            grow_buffer(self, elems)
        end
        if _type(b) == "number" then
            self.bytes[index] = b
        else
            for i=1, #b do
                self.bytes[index + i - 1] = b[i]
            end
        end
        self.size = self.size + elems
    end,
    remove=function(self, index, elems)
        if index <= 0 or index > self.size then
            return
        end
        if elems == nil then
            elems = 1
        end
        if index + elems > self.size then
            elems = self.size - index + 1
        end
        for i=index, self.size - elems do
            self.bytes[i] = self.bytes[i + elems]
        end
        self.size = self.size - elems
    end,
    clear=function(self)
        self.size = 0
    end,
    reserve=function(self, new_capacity)
        if new_capacity <= self.capacity then
            return
        end
        local prev = self.bytes
        self.bytes = malloc(new_capacity)
        FFI.copy(self.bytes, prev, self.size)
        self.capacity = new_capacity
        free(prev)
    end,
}

local bytearray_mt = {
    __index = function(self, key)
        if _type(key) == "string" then
            return bytearray_methods[key]
        end
        if key <= 0 or key > self.size then
            return
        end
        return self.bytes[key - 1]
    end,
    __newindex = function(self, key, value)
        if key <= 0 or key > self.size then
            return
        end
        self.bytes[key - 1] = value
    end,
    __tostring = function(self)
        return string.format("FFIBytearray[%s]{...}", tonumber(self.size))
    end,
    __len = function(self)
        return tonumber(self.size)
    end,
    __gc = function(self)
        free(self.bytes)
    end
}

local utf8tostring = utf8.tostring

function utf8.tostring(bytes)
    local type = _type(bytes)
    if type == "cdata" then
        return FFI.string(bytes.bytes, bytes.size)
    else
        return utf8tostring(bytes)
    end
end

local bytearray_type = FFI.metatype("bytearray_t", bytearray_mt)

return function (n)
    n = n or 0
    if n < MIN_CAPACITY then
        return bytearray_type(malloc(MIN_CAPACITY), n, MIN_CAPACITY)
    else
        return bytearray_type(malloc(n), n, n)
    end
end
