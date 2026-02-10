<div align="center">
  <img src="logo.png" alt="IAGPU Logo" width="130" style="border-radius: 1.15rem;"/>
  <br/>
  
  <img src="https://img.shields.io/badge/license-apache_v2-blue.svg" alt="License"/>
  <img src="https://img.shields.io/badge/standard-C%2B%2B20-yellow.svg" alt="C++ Standard"/>

  <p style="padding-top: 0.2rem;">
    <b></b>
  </p>
</div>

## **Description**

## **Building**

IAGPU uses `CMakePresets.json` for build configuration.

**Linux (x64)**

```bash
cmake --preset iagpu-x64-linux  
cmake --build --preset iagpu-x64-linux  
ctest --preset iagpu-x64-linux
```

**Windows (x64 Clang)**

```bash
cmake --preset iagpu-x64-windows  
cmake --build --preset iagpu-x64-windows  
ctest --preset iagpu-x64-windows
```

## **License**

Copyright (C) 2026 IAS. Licensed under the [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).
