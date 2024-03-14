local vector2 = {__type = "vec2"}
local vector3 = {__type = "vec3"}

vector2.__index = vector2
vector3.__index = vector3

vec2 = function(x, y) return vector2:new(x, y) end
vec3 = function(x, y, z) return vector3:new(x, y, z) end

-- vec2 cocnrtuct
function vector2:new(x, y)
    if type(x) ~= "number" or type(y) ~= "number" then
        error("Invalid input argument. Expected two numbers. [ vec2(number, number) ]")
    end

    local obj = {
        x = x or 0,
        y = y or 0
    }

    setmetatable(obj, self)
    return obj
end

-- string vec
function vector2:strvec2()
    return "(" .. self.x .. ", " .. self.y ..  ")"
end

-- round vector components
-- @param decimals -> integer
-- @return round(vec2)
-- @usage:
--      @example-1:
--              local v1 = vec2(1, 0.5)
--              print(v1:round()) -- Output: (1, 1)
--      @example-2:
--              local v1 = vec2(0.707, 0.5)
--              print(v1:round(2)) -- Output: (0.71, 0.5)
function vector2:round(decimals)
    decimals = decimals or 0
    self.x = math.floor(self.x * 10^decimals + 0.5) / 10^decimals
    self.y = math.floor(self.y * 10^decimals + 0.5) / 10^decimals
    return self
end


-- vec2 magnitude (length)
-- @return {number} - the magnitude of the vector
-- @usage:
--      @example-1:
--              local v1 = vec2(-10, 25)
--              print(v1:len()) -- Output: 26.925824035673
--      @example-2:
--              local v1 = vec2(0.32, 89)
--              print(v1:len()) -- Output: 89.
function vector2:len() 
    return math.sqrt(self.x * self.x + self.y * self.y)
end


-- Normalize vec2
-- @return {vector2} - The normalized vector2
-- @usage:
--      @example-1:
--              local v1 = vec2(10, 15)
--              print(v1:norm()) -- Output: (0.55470019622523, 0.83205029433784)
--      @example-2:
--              local v1 = vec2(-1, 1)
--              print(v1:norm()) -- Output: (-0.70710678118655, 0.70710678118655)
function vector2:norm()
    local length_vectors = self:len()
    return vector2:new(self.x / length_vectors, self.y / length_vectors)
end


-- vec2 dot product
-- @param vector {vec2} 
-- @return {number}
-- @usage:
--      @example-1:
--              local v1 = vec2(10, 15)
--              local v2 = vec2(-1, 1)
--              print(v1:dot(v2)) -- Output: 5
--      @example-2:
--              local v1 = vec2(-15, 10)
--              print(v1:dot(v1)) -- Output: (-0.83205029433784, 0.55470019622523)
function vector2:dot(vector)
    if type(vector) == "number" then
        error("Invalid input argument. Expected a vector2 object")
    end
    return self.x * vector.x + self.y * vector.y
end


-- Linear interpolation for vector2
-- @param b {vector2} - The target vector2
-- @param t {number} - (0 <= t <= 1) the interpolation factor
-- @return {vector2} - The interpolated vector2
function vector2:lerp(b, t)
    if type(b) ~= "vector2" then
        error("Invalid input argument. Expected a vector2 object")
    end
    if type(t) ~= "number" or t < 0 or t > 1 then
        error("Invalid input argument. Expected a number between 0 and 1")
    end
    return vector2:new(self.x + t * (b.x - self.x), self.y + t * (b.y - self.y));
end

-- rotate vec2
-- @param angle {number}
-- @param axis {string} - axis rotate around (x, y, or z)
-- @param convert2deg {bool} .. if true => deg convert to rad 
-- @return {vector2} - rotated vector2
function vector2:rot(angle, axis, convert2deg)
    if convert2deg == true then 
        angle = math.rad(angle) 
    end

    if type(axis) == "string" then
        if axis == "x" then 
            local x_new = self.x
            local y_new = self.y * math.cos(angle) - self.y * math.sin(angle)
            self.y = y_new
        elseif axis == "y" then
            local x_new = self.x * math.cos(angle) + self.x * math.sin(angle)
            local y_new = self.y
            self.x = x_new
        elseif axis == "z" then
            local x_new = self.x * math.cos(angle) - self.y * math.sin(angle)
            local y_new = self.x * math.sin(angle) + self.y * math.cos(angle)
            self.x, self.y = x_new, y_new
        end
    elseif axis == nil then
        local x_new = self.x * math.cos(angle) - self.y * math.sin(angle)
        local y_new = self.x * math.sin(angle) + self.y * math.cos(angle)
        self.x, self.y = x_new, y_new
    end
    return self:round(15)
end

-- add vec 
function vector2.__add(value_1, value_2)
    if type(value_1) == "number" then 
        if value_1 == 0 then 
            return vector2:new(value_2.x, value_2.y)
        else 
            return vector2:new(value_2.x + value_1, value_2.y + value_1)
        end
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return vector2:new(value_1.x, value_1.y)
            else
                return vector2:new(value_1.x + value_2, value_1.y + value_2)
            end
        else
            return vector2:new(value_1.x + value_2.x, value_1.y + value_2.y)
        end
    end
end



-- mul vec
function vector2.__mul(value_1, value_2)
    if type(value_1) == "number" then
        return vector2:new(value_2.x * value_1, value_2.y * value_1)
    else
        if type(value_2) == "number" then
            return vector2:new(value_1.x * value_2, value_1.y * value_2)
        else
            return vector2:new(value_1.x * value_2.x, value_1.y * value_2.y)
        end
    end
end

-- pow vec
function vector2.__pow(value_1, value_2)
    if type(value_1) == "number" then
        return vector2:new(value_1.x ^ value_2, value_2.y ^ value_1)
    else
        if type(value_2) == "number" then
            return vector2:new(value_1.x ^ value_2, value_1.y ^ value_2)
        else
            return vector2:new(value_1.x ^ value_2.x, value_1.y ^ value_2.y)
        end
    end
end



-- div vec
function vector2.__div(value_1, value_2)
    if type(value_1) == "number" then
        return vector2:new(value_2.x / value_1, value_2.y / value_1)
    else
        if type(value_2) == "number" then
            return vector2:new(value_1.x / value_2, value_1.y / value_2)
        else
            return vector2:new(value_1.x / value_2.x, value_1.y / value_2.y)
        end
    end
end


-- eq vec
function vector2.__eq(a, b)
    return a.x == b.x and a.y == b.y
end


-- eq vec
function vector2.__eq(a, b)
    return a.x == b.x and a.y == b.y
end

function vector2.__tostring(vcrt)
    return vcrt:strvec2()
end


-- vec3 construct
function vector3:new(x, y, z)
    if type(x) ~= "number" or type(y) ~= "number" or type(z) ~= "number" then
        print("\n(( TypeError : new vec3))\nType arg vec3(x, y, z) = " .. "(" .. type(x) .. ", " .. type(y) .. ", " .. type(z) .. ")")
        error("Invalid input argument. Expected a vector obj. [ vec3(scalar, scalar, scalar) ]\n\n")
    end
    local obj = {
      x = x or 0,
      y = y or 0,
      z = z or 0
    }
    setmetatable(obj, self)
    return obj
  end

-- str vec
function vector3:strvec3()
    return "(" .. self.x .. ", " .. self.y .. ", " .. self.z .. ")"
end
  
-- dot product
-- @param {vec3}
-- @return {number} -> float || integer
-- @usage:
--     local v1 = vec3(10, 15)
--     local v2 = vec3(15, 10, 1)
--     print(v1:dot(v2)) -- Output: 301
function vector3:dot(v)
    if type(v) == "number" then
        print("\n(( TypeError : dot ))\nType arg dot(vector3), your arg: " .. "dot(" .. type(v) .. ")")
        error("Invalid input argument. Expected a vector3 obj.\n")
    end
    return self.x * v.x + self.y * v.y + self.z * v.z
end
  
-- cross product
function vector3:cross(v)
    if type(v) == "number" then
        print("\n(( TypeError : cross ))\nType arg cross(vector3), your arg: " .. "cross(" .. type(v) .. ")")
        error("Invalid input argument. Expected a vector3 obj.\n")
    end
    return vector3:new(
      self.y * v.z - self.z * v.y,
      self.z * v.x - self.x * v.z,
      self.x * v.y - self.y * v.x
    )
end
  
-- round vector components
function vector3:round(decimals)
    decimals = decimals or 0
    self.x = math.floor(self.x * 10^decimals + 0.5) / 10^decimals
    self.y = math.floor(self.y * 10^decimals + 0.5) / 10^decimals
    self.z = math.floor(self.z * 10^decimals + 0.5) / 10^decimals
    return self
end
  

-- check if two vectors are parallel
-- @param {vec3}
-- @return {bool}
-- @usage:
--      local v1 = vec3(10, 15, 1)
--      local v2 = vec3(10, 15, 1)
--      print(v1:isParallel(v2)) -- Output: true
function vector3:isParallel(val)
    if type(val) ~= "table" or type(val) == "number" then
        print("\n(( TypeError : isParallel ))\nType arg isParallel(vector3)")
        error("Invalid input argument. Expected a vector3 object.\n")
    end
    return self:cross(val):len() < 1e-6
end

-- magnitude (length) vec
-- @return {number}
function vector3:len()
    return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)
end



-- normalize vector
-- @return {number}
-- @usage:
--      local v1 = vec3(10, 15, 1)
--      print(v1:norm()) -- Output: 18.1
function vector3:norm()
    local length_vectors = self:len()
    return vector3:new(self.x / length_vectors, self.y / length_vectors, self.z / length_vectors)
end


-- linear interpolation (lerp) for vector3
-- @param b {vec3} - The target vector3 object
-- @param t {number} - (0 <= t <= 1) the interpolation factor
-- @return {vec3}
function vector3:lerp(b, t)
    if type(b) ~= "vector3" then
        error("Invalid input argument. Expected a vector3 object")
    end
    if type(t) ~= "number" or t < 0 or t > 1 then
        error("Invalid input argument. Expected a number between 0 and 1")
    end
    return vector3:new(self.x + t * (b.x - self.x), self.y + t * (b.y - self.y), self.z + t * (b.z - self.z));
end


-- rot vec3
-- @param angle {number} -> float || int
-- @param axis {string}  .. if nil => rot axis z
-- @param convert2deg {bool} .. if true => deg convert to rad
-- @return {vec3}
function vector3:rot(angle, axis, convert2deg)
    if convert2deg == true then
        angle = math.rad(angle)
    end

    if type(axis) == "string" then
        local cx, sx = math.cos(angle), math.sin(angle)
        if axis == "x" then
            local x, y, z = self.x, self.y, self.z
            self.y = cx * y - sx * z
            self.z = sx * y + cx * z
        elseif axis == "y" then
            local x, y, z = self.x, self.y, self.z
            self.x = cx * x + sx * z
            self.z = -sx * x + cx * z
        elseif axis == "z" then
            local x, y, z = self.x, self.y, self.z
            self.x = cx * x - sx * y
            self.y = sx * x + cx * y
        end
    elseif axis == nil then
        local cosx, sx, cy, sy = math.cos(angle), math.sin(angle), math.cos(angle), math.sin(angle)
        local x, y, z = self.x, self.y, self.z
        self.x = cx * x - sx * y
        self.y = sx * x + cx * y
        self.z = cy * z - sy * self:len()
        local len = self:len()
        if len ~= 0 then
            self.z = self.z / len
        end
    end
    return self:round(15)
end


-- add vec  
function vector3.__add(value_1, value_2)
    if type(value_1) == "number" then
        if value_1 == 0 then
            return value_2
        end
        return vector3:new(value_2.x + value_1, value_2.y + value_1, value_2.z + value_1)
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return value_1
            end
            return vector3:new(value_1.x + value_2, value_1.y + value_2, value_1.z + value_2)
        else
            if value_1.z ~= nil and value_2.z ~= nil then
                return vector3:new(value_1.x + value_2.x, value_1.y + value_2.y, value_1.z + value_2.z)
            else
                return vector3:new(value_1.x + value_2.x, value_1.y + value_2.y)
            end
        end
    end
end





function vector3.__sub(value_1, value_2)
    if type(value_1) == "number" then
        if value_1 == 0 then
            return value_2
        end
        return vector3:new(value_2.x - value_1, value_2.y - value_1, value_2.z - value_1)
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return value_1
            end
            return vector3:new(value_1.x - value_2, value_1.y - value_2, value_1.z - value_2)
        else
            if value_1.z ~= nil and value_2.z ~= nil then
                return vector3:new(value_1.x - value_2.x, value_1.y - value_2.y, value_1.z - value_2.z)
            else
                return vector3:new(value_1.x - value_2.x, value_1.y - value_2.y)
            end
        end
    end
end





-- mul vec
function vector3.__mul(value_1, value_2)
    if type(value_1) == "number" then
        if value_1 == 0 then
            return value_2
        end
        return vector3:new(value_2.x * value_1, value_2.y * value_1, value_2.z * value_1)
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return value_1
            end
            return vector3:new(value_1.x * value_2, value_1.y * value_2, value_1.z * value_2)
        else
            if value_1.z ~= nil and value_2.z ~= nil then
                return vector3:new(value_1.x * value_2.x, value_1.y * value_2.y, value_1.z * value_2.z)
            else
                return vector3:new(value_1.x * value_2.x, value_1.y * value_2.y)
            end
        end
    end
end

function vector3.__pow(value_1, value_2)
    if type(value_1) == "number" then
        if value_1 == 0 then
            return value_2
        end
        return vector3:new(value_2.x ^ value_1, value_2.y ^ value_1, value_2.z ^ value_1)
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return value_1
            end
            return vector3:new(value_1.x ^ value_2, value_1.y ^ value_2, value_1.z ^ value_2)
        else
            if value_1.z ~= nil and value_2.z ~= nil then
                return vector3:new(value_1.x ^ value_2.x, value_1.y ^ value_2.y, value_1.z ^ value_2.z)
            else
                return vector3:new(value_1.x ^ value_2.x, value_1.y ^ value_2.y)
            end
        end
    end
end



-- div vec
function vector3.__div(value_1, value_2)
    if type(value_1) == "number" then
        if value_1 == 0 then
            return value_2
        end
        return vector3:new(value_2.x / value_1, value_2.y / value_1, value_2.z / value_1)
    else
        if type(value_2) == "number" then
            if value_2 == 0 then
                return value_1
            end
            return vector3:new(value_1.x / value_2, value_1.y / value_2, value_1.z / value_2)
        else
            if value_1.z ~= nil and value_2.z ~= nil then
                return vector3:new(value_1.x / value_2.x, value_1.y / value_2.y, value_1.z / value_2.z)
            else
                return vector3:new(value_1.x / value_2.x, value_1.y / value_2.y)
            end
        end
    end
end




-- eq vec
function vector3.__eq(a, b)
    return a.x == b.x and a.y == b.y and a.z == b.z
end



function vector3.__tostring(vcrt3)
    return vcrt3:strvec3()
end

