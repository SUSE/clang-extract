struct AA {
  int key;
};

int g(void);

extern struct AA aaa;

#define STATIC_KEY_FALSE_INIT (struct AA){ .key = 0, }

#define DEFINE_STATIC_KEY_FALSE(name)	\
	struct AA name = STATIC_KEY_FALSE_INIT

