#ifndef COMMON_INCLUDED
#define COMMON_INCLUDED

#define GLM_FORCE_CXX14
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_UNRESTRICTED_GENTYPE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/norm.hpp>

const int WIDTH = 1280;
const int HEIGHT = 720;
const int MAX_POINTS = 9;
const int MAX_DEPTH = 7;
const double PADDING = 20.0;

const glm::vec3 black(0.157, 0.173, 0.204);
const glm::vec3 white(0.671, 0.698, 0.749);
const glm::vec3 lightRed(0.878, 0.424, 0.459);
const glm::vec3 darkRed(0.745, 0.314, 0.275);
const glm::vec3 green(0.596, 0.765, 0.475);
const glm::vec3 lightYellow(0.898, 0.753, 0.482);
const glm::vec3 darkYellow(0.82, 0.604, 0.4);
const glm::vec3 blue(0.38, 0.686, 0.937);
const glm::vec3 magenta(0.776, 0.471, 0.867);
const glm::vec3 cyan(0.337, 0.714, 0.761);
const glm::vec3 gray(0.361, 0.388, 0.439);

#endif
