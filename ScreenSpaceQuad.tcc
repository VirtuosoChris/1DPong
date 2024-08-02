
inline ScreenSpaceQuad::~ScreenSpaceQuad(){
	std::cout<<"Screenspace quad destructor\n";
	glDeleteBuffers(1, &quadVBO);
}


inline const GLuint& ScreenSpaceQuad::getID()const{

	return quadVBO;
}

 inline const ScreenSpaceQuad& ScreenSpaceQuad::getInstance(){
//  std::cout<<"Screenspace quad getInstance\n";
  static ScreenSpaceQuad* ins = NULL;
  if(!ins)ins = new ScreenSpaceQuad();

  return *ins;
}


inline void ScreenSpaceQuad::render(){
//	std::cout<<"Screenspace quad render\n";
ScreenSpaceQuad::getInstance().draw();
}



inline ScreenSpaceQuad& ScreenSpaceQuad::operator=(const ScreenSpaceQuad&){
		throw new std::string("screen space quad assignment operator called: this shouldn't happen");
}

inline ScreenSpaceQuad::ScreenSpaceQuad(const ScreenSpaceQuad&){
		throw new std::string("screen space quad copy ctor called: this shouldn't happen");
}
