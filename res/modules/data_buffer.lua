local bit_converter = require "core:bit_converter"

local MAX_UINT16 = 65535
local MIN_UINT16 = 0
local MAX_UINT32 = 4294967295
local MIN_UINT32 = 0

local MAX_INT16 = 32767
local MIN_INT16 = -32768
local MAX_INT32 = 2147483647
local MIN_INT32 = -2147483648
local MAX_INT64 = 9223372036854775807
local MIN_INT64 = -9223372036854775808

local TYPE_ZERO = 0
local TYPE_UINT16 = 1
local TYPE_UINT32 = 2
local TYPE_INT16 = 3
local TYPE_INT32 = 4
local TYPE_INT64 = 5
local TYPE_DOUBLE = 6

-- Data buffer

local data_buffer =
{
	__call =
	function(data_buffer, bytes)
		return data_buffer:new(bytes)
	end
}

function data_buffer:new(bytes)
    local obj = {
        pos = 1,
        bytes = bytes or { }
    }

    self.__index = self
    setmetatable(obj, self)
    print("asg")
    return obj
end

-- Push functions

function data_buffer:put_byte(byte)
	if byte < 0 or byte > 255 then
		error("invalid byte")
	end

	self.bytes[self.pos] = byte

	self.pos = self.pos + 1
end

function data_buffer:put_bytes(bytes)
	for i = 1, #bytes do
		self:put_byte(bytes[i])
	end
end

function data_buffer:put_single(single)
	self:put_bytes(bit_converter.single_to_bytes(single))
end

function data_buffer:put_double(double)
	self:put_bytes(bit_converter.double_to_bytes(double))
end

function data_buffer:put_string(str)
	self:put_bytes(bit_converter.string_to_bytes(str))
end

function data_buffer:put_bool(bool)
	self:put_byte(bit_converter.bool_to_byte(bool))
end

function data_buffer:put_uint16(uint16)
	self:put_bytes(bit_converter.uint16_to_bytes(uint16))
end

function data_buffer:put_uint32(uint32)
	self:put_bytes(bit_converter.uint32_to_bytes(uint32))
end

function data_buffer:put_int16(int16)
	self:put_bytes(bit_converter.int16_to_bytes(int16))
end

function data_buffer:put_int32(int32)
	self:put_bytes(bit_converter.int32_to_bytes(int32))
end

function data_buffer:put_int64(int64)
	self:put_bytes(bit_converter.int64_to_bytes(int64))
end

function data_buffer:put_number(num)
	local bytes
	local type

	if math.floor(num) ~= num then
		type = TYPE_DOUBLE
		bytes = bit_converter.double_to_bytes(num)
	elseif num == 0 then
		type = TYPE_ZERO
		bytes = { }
	elseif num > 0 then
		if num <= MAX_UINT16 then
			type = TYPE_UINT16
			bytes = bit_converter.uint16_to_bytes(num)
		elseif num <= MAX_UINT32 then
			type = TYPE_UINT32
			bytes = bit_converter.uint32_to_bytes(num)
		elseif num <= MAX_INT64 then
			type = TYPE_INT64
			bytes = bit_converter.int64_to_bytes(num)
		end
	elseif num < 0 then
		if num >= MIN_INT16 then
			type = TYPE_INT16
			bytes = bit_converter.int16_to_bytes(num)
		elseif num >= MIN_INT32 then
			type = TYPE_INT32
			bytes = bit_converter.int32_to_bytes(num)
		elseif num >= MIN_INT64 then
			type = TYPE_INT64
			bytes = bit_converter.int64_to_bytes(num)
		end
	end

	self:put_byte(type)
	self:put_bytes(bytes)
end

-- Get functions

function data_buffer:get_byte()
	local byte = self.bytes[self.pos]
	self.pos = self.pos + 1
	return byte
end

function data_buffer:get_number()
	local type = self:get_byte()

	if type == TYPE_ZERO then
		return 0
	elseif type == TYPE_UINT16 then
		return self:get_uint16()
	elseif type == TYPE_UINT32 then
		return self:get_uint32()
	elseif type == TYPE_INT16 then 
		return self:get_int16()
	elseif type == TYPE_INT32 then 
		return self:get_int32()
	elseif type == TYPE_INT64 then 
		return self:get_int64()
	elseif type == TYPE_DOUBLE then
		return self:get_double()
	else
		error("unknown lua number type: "..type)
	end
end

function data_buffer:get_single()
	return bit_converter.bytes_to_single(self:get_bytes(4))
end

function data_buffer:get_double()
	return bit_converter.bytes_to_double(self:get_bytes(8))
end

function data_buffer:get_string()
	local len = self:get_bytes(2)
	local str = self:get_bytes(bit_converter.bytes_to_uint16(len))
	local bytes = { }

	for i = 1, #len do
		bytes[i] = len[i]
	end

	for i = 1, #str do
		bytes[#bytes + 1] = str[i]
	end

	return bit_converter.bytes_to_string(bytes)
end

function data_buffer:get_bool()
	return bit_converter.byte_to_bool(self:get_byte())
end

function data_buffer:get_uint16()
	return bit_converter.bytes_to_uint16(self:get_bytes(2))
end

function data_buffer:get_uint32()
	return bit_converter.bytes_to_uint32(self:get_bytes(4))
end

function data_buffer:get_int16()
	return bit_converter.bytes_to_int16(self:get_bytes(2))
end

function data_buffer:get_int32()
	return bit_converter.bytes_to_int32(self:get_bytes(4))
end

function data_buffer:get_int64()
	return bit_converter.bytes_to_int64(self:get_bytes(8))
end

function data_buffer:size()
	return #self.bytes
end

function data_buffer:get_bytes(n)
	if n == nil then
		return self.bytes
	else
		local bytes = { }

		for i = 1, n do
			bytes[i] = self:get_byte()
		end

		return bytes
	end
end

function data_buffer:set_position(pos)
	self.pos = pos
end

function data_buffer:set_bytes(bytes)
	for i = 1, #bytes do
		local byte = bytes[i]
		if byte < 0 or byte > 255 then
			error("invalid byte")
		end
	end

	self.bytes = bytes
end

setmetatable(data_buffer, data_buffer)

return data_buffer