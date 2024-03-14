local vector2 = {__type = "vec2"}
local vector3 = {__type = "vec3"}

vector2.__index = vector2
vector3.__index = vector3

vec2 = function(x, y) return vector2:new(x, y) end
vec3 = function(x, y, z) return vector3:new(x, y, z) end


function vector2:new(x, y)
    local obj = {
        x = x or 0,
        y = y or 0
    }
    setmetatable(obj, self)
    return obj
end

-- str vec
function vector2:strvec2()
    return "(" .. self.x .. ", " .. self.y ..  ")"
end

-- magnitude (length) vec
function vector2:len()
    return math.sqrt(self.x * self.x + self.y * self.y)
end

-- normalize vector
function vector2:norm()
  local length_vectors = self:len()
  return vector2:new(self.x / length_vectors, self.y / length_vectors)
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
  
  -- dot product
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
function vector3:is_parallel(v)
    if type(v) ~= "vector3" then
      error("Invalid input argument. Expected a vector3 object.")
    end
    return self:cross(v):len() < 1e-6
end


-- string vec
function vector3:strvec3()
    return "(" .. self.x .. ", " .. self.y .. ", " .. self.z .. ")"
end



-- magnitude (length) vec
function vector3:len()
    return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z)
end



-- normalize vector
function vector3:norm()
  local length_vectors = self:len()
  return vector3:new(self.x / length_vectors, self.y / length_vectors, self.z / length_vectors)
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




