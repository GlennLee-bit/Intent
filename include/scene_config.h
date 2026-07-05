#ifndef SCENE_CONFIG_H
#define SCENE_CONFIG_H

#include "intent_types.h"

int SceneConfigLoadFromJson(const char *JsonString, SceneConfig *Config);
void SceneConfigDestroy(SceneConfig *Config);
int SceneConfigLoadFromFile(const char *FilePath, SceneConfig *Config);

#endif /* SCENE_CONFIG_H */