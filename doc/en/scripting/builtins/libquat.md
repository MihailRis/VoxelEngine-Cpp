# *quat* library

Quaternions manipulation library.

## Quaternion from matrix - *mat4.from_quat(...)*

```lua
-- creates a quaternion based on the rotation matrix
quat.from_mat4(m: matrix)

-- writes a quaternion from the rotation matrix to dst
quat.from_mat4(m: matrix, dst: quat)
```

## Spherical linear interpolation - *quat.slerp(...)*

The interpolation always take the short path and the rotation is performed at constant speed.

```lua
-- creates a quaternion as an interpolation between a and b,
-- where t is interpolation factor
quat.slerp(a: quat, b: quat, t: number)

-- writes a quaternion as an interpolation between a and b to dst,
-- where t is interpolation factor
quat.slerp(a: quat, b: quat, t: number, dst: quat)
```

## Casting to string - *quat.tostring(...)*

```lua
-- returns a string representing the contents of the quaternion
quat.tostring(q: quat)
```
