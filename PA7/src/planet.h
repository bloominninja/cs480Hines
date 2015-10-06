#ifndef PLANET_H_
#define PLANET_H_

typedef class Planet Planet;

struct Moon
{
 int distance;
 Planet th;
};

class Planet
    {

    public:
    Planet();
    ~Planet();
    void GetPlanet(char*);
    void UpdatePlanet();
    void UpdateMoons();
    glm::mat4* planet;
	    
    private:
	int rotate;
	
	glm::mat4 scaledPos;
	Moon moons[4];
	int numMoons;
	
};

#endif
