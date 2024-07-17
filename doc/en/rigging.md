# Rigging

## Skeletons

Entity skeletons are created via json files in the *skeletons* folder.

> [!IMPORTANT]
>
> The skeleton is a non-indexable content unit. When loading it, the pack prefix is ​​added to the name (example: *drop* in the base pack -> *base:drop*).

A skeletal element, or bone, consists of a transformation matrix defining its position, rotation and scale relative to the parent element (bone) or entity if the element is the root, a model and a list of sub-elements.

A skeleton file has the following structure:
```json
{
    "root": {
        "name": "name",
        "model": "model_name",
        "nodes": [
            ...
        ]
    }
}
```

- root - root element
- name - name of the element to get the index (field optional)
- model - name of the model to display the element (field optional)
- nodes - list of elements - descendants, which are affected by the matrix of this element (the field is optional)

At the moment, positioning, rotation, scaling is done through scripting, as well as animation.

The process of working with skeletons will be simplified in the future.

Models are loaded automatically; adding them to preload.json is not required.

## Models

Models should be located in the models folder. Currently only OBJ format is supported.

>[!IMPORTANT]
> When loading an obj model, the \*.mtl file is ignored.

 A texture is defined by a material name that matches the texture naming format used in preload.json.

 Textures are loaded automatically; it is not necessary to specify the textures used by the model in preload.json.
