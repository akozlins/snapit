
/*
 * This file is part of 'snapit' program.
 *
 * Copyright (c) 2012 Alexandr Kozlinskiy
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
