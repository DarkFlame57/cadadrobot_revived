#ifndef __CONFIG_H__
#define __CONFIG_H__

class Config {
  public:
    Config(String file_name);
    String get(String key);
  private:
    String file_name;
};

#endif /* ifndef __CONFIG_H__ */
