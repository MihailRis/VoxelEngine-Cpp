# Resources

Resources include:
- cameras
- effects slots
- framebuffers
- and other limited resources

At the moment only **cameras** are implemented.

The resources requested by the pack are specified through the *resources.json* file in the format:
```json
{
    "resource-type": [
        "resources",
        "names"
    ]
}
```

After loading the pack, resource names will have the pack prefix. For example camera
*cinematic* in the base package will be *base:cinematic*.
