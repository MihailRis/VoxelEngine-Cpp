# *world* library

```lua
-- Returns worlds information.
world.get_list() -> tables array {
    -- world name
    name: str,
    -- world icon/preview (loading automatically)
    icon: str
}

-- Returns current day time in range \[0.0-1.0\] 
-- where 0.0 and 1.0 - midnight, 0.5 - noon.
world.get_day_time() -> number

-- Set day time value.
world.set_day_time(time: number)

-- Sets the specified day time cycle speed.
world.set_day_time_speed(value: number)

-- Returns the day time cycle speed.
world.get_day_time_speed() -> number

-- Returns total time passed in the world.
world.get_total_time() -> number

-- Returns world seed.
world.get_seed() -> int

-- Returns generator name.
world.get_generator() -> str

-- Proves that this is the current time during the day 
-- from 0.333(8 am) to 0.833(8 pm).
world.is_day() -> boolean

-- Checks that it is the current time at night
-- from 0.833(8 pm) to 0.333(8 am).
world.is_night() -> bool

-- Checks the existence of a world by name.
world.exists() -> bool
```
