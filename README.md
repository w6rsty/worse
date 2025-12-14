# Worse Engine

Rendering engine. 

!!! REFACTORING WIP !!!

# Generate project
Use tool script
```sh
./Scripts/utt.sh generate test
```

## Type Naming Convention
| Prefix  | Meaning |
| --- | ------------ |
| `E` | enum |
| `I` | Pure interface |
| `H` | Handle |
| `R` | Reference-counted handle |
| `T` | Template/Generic container |
| `F` | Behaviour type |
| `V` | Trivial value type |


## Vulkan Extensions
- VK_EXT_descriptor_buffer
- VK_EXT_descriptor_indexing
- VK_EXT_buffer_device_address
- VK_KHR_synchronization2