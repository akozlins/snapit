
struct iniconf
{
  char* map[16][2];

  void ctor(const char* fname)
  {
    memset(map, 0, sizeof(map));

    char appbuf[1024], keybuf[1024], valbuf[1024];

    int i = 0;

    GetPrivateProfileString(NULL, NULL, NULL, appbuf, 1024, fname);
    for(char* app = appbuf; *app;)
    {
      int applen = strlen(app) + 1; // <-

      GetPrivateProfileString(app, NULL, NULL, keybuf, 1024, fname);
      for(char* key = keybuf; *key;)
      {
        int keylen = strlen(key) + 1; // <-

        map[i][0] = (char*)malloc(applen + 1 + keylen);
        strcpy_s(map[i][0], applen + keylen, app);
        strcat_s(map[i][0], applen + keylen, ".");
        strcat_s(map[i][0], applen + keylen, key);
        int vallen = GetPrivateProfileString(app, key, NULL, valbuf, 1024, fname) + 1; // <-
        map[i][1] = (char*)malloc(vallen);
        strcpy_s(map[i][1], vallen, valbuf);
        i++;

        key += keylen;
      }

      app += applen;
    }
  }

  void dtor()
  {
    for(int i = 0; i < 16; i++)
    {
      if(map[i][0]) free(map[i][0]);
      if(map[i][1]) free(map[i][1]);
    }
  }
};
