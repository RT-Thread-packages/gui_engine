# GUI Engine

[中文页](README_ZH.md) | English

## 1. Introduction

GUI engine is a set of basic drawing engine, written by C code.

The main functions include:
- Drawing operations based on the drawing device DC, including points, lines, circles, ellipses, polygons (filling), etc.;
- Various image format loading (loading from file system, DFS file system is required) and drawing;
- Text display in various fonts;
- GUI C/S structure and basic window mechanism, event frame mechanism, etc.

## 2. How to obtain

- Obtain by Git:

     git clone https://github.com/RT-Thread-packages/gui_engine.git

- env tool to assist download:

menuconfig package path:

     RT-Thread online package
         system package
             [*] RT-Thread UI Engine
                 [*] Enable UI Engine

## 3. Example introduction

### 3.1 Get examples

* Configure the enable example option `Enable the example of Gui Engine`
* The configuration package version is selected as the latest version `latest`
