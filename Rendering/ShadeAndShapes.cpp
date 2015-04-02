#include "Common/Common.h" 
#include "Rendering/ShadeAndShapes.h" 


void Sphere::translate(const Vec3& trans){
	_center+=trans;
	updateTransform(); 
}

// rotate a sphere has no effect
void Sphere::rotate(double r, int axis){

}

void Box::translate(const Vec3& trans){
	_corner+=trans;
	updateTransform(); 
}

void Box::rotate(double d, int axis){ 
	// the rotation code for the other objects should look very similar.
	// first construct a rotation about x/y/z axis
	std::auto_ptr<Mat4> m;
	m =  GeometryUtils::getRotateMatrix(d, axis);
	Vec3 cv = _corner - _center; 

	// rotate the three linearly independent vectors that determine the shape
	_lengthv = _lengthv * (*m);
	_widthv = _widthv * (*m);
	_heightv = _heightv * (*m);

	// rotation is about the center, so points that are not the center will also be rotated; 
	_corner = _center + (cv * (*m)); 

	updateTransform(); 
}
// TODO: fill in these functions
void Cylinder::translate(const Vec3& trans) {
	_baseCenter+=trans;
	updateTransform();
}
void Cylinder::rotate(double d, int axis) {
	std::auto_ptr<Mat4> m;
	m =  GeometryUtils::getRotateMatrix(d, axis);

	_baseAxis1 = _baseAxis1 * (*m);
	_baseAxis2 = _baseAxis2 * (*m);
	_centerAxis = _centerAxis * (*m);

	updateTransform(); 
}
void Cone::translate(const Vec3& trans) {
	_baseCenter+=trans;
	updateTransform();
}
void Cone::rotate(double d, int axis) {
	std::auto_ptr<Mat4> m;
	m =  GeometryUtils::getRotateMatrix(d, axis);

	_baseAxis1 = _baseAxis1 * (*m);
	_baseAxis2 = _baseAxis2 * (*m);
	_centerAxis = _centerAxis * (*m);

	updateTransform(); 
}
void Ellipsoid::translate(const Vec3& trans) {
	_center+=trans;
	updateTransform();
}
void Ellipsoid::rotate(double d, int axis) {
	std::auto_ptr<Mat4> m;
	m =  GeometryUtils::getRotateMatrix(d, axis);

	_axis1 = _axis1 * (*m);
	_axis2 = _axis2 * (*m);
	_axis3 = _axis3 * (*m);

	updateTransform(); 
}

// definition of the sphere can be pretty sparse.  
//You don't need to define the transform associated with a sphere.
void Sphere::updateTransform() {

}

/**
 * @brief calculate the first interction between a ray and a sphere
 * @details implement the algorithm in the book
 * 
 * @param sphere a sphere
 * @param ret the result struct of intersection
 */
void Intersector::visit(Sphere* sphere, void* ret){
	// TODO: rewrite this as described in the textbook
	IsectData* iret = (IsectData*) ret; 

	Pt3 center = sphere->getCenter(); 
	double radius = sphere->getRadius(); 

	double closest = GeometryUtils::pointRayClosest(center,_ray); 
    Vec3 C2P = _ray.at(closest) - center; 
    double r2 = radius*radius; 
	double d2 = C2P*C2P; 

	if(d2 > r2 + EPS){
        iret->hit = false; 
		iret->t0 = 0; 
	}
	else{
		double h = sqrt(r2 - d2); 
		if (closest+h<0){
			iret->hit = false;
			return;
		} else if (closest-h<0.001){
			iret->t0 = closest+h; 
			iret->hit = true;
		} else {
			iret->t0 = closest-h; 
			iret->hit = true; 
		}
		iret->normal = (_ray.at(iret->t0) - center); 
		iret->normal.normalize(); 
	}
}

// The updateTransform functions should properly set up the transformation of this geometry.
// The transformation takes a unit shape into the shape described by the parameters.
// This function also computes the inverse of the transformation.
void Box::updateTransform(){
	Vec3 ncenter = _corner; 
	ncenter += _length/2 * _lengthv; 
	ncenter += _width/2 * _widthv; 
	ncenter += _height/2 * _heightv; 
	_center = ncenter; 
	_mat = compose(_lengthv * _length, _widthv * _width, _heightv * _height, _center);
	_imat = !_mat; 

	Geometry::updateTransform(); // must call this at the end
}

// a box has six faces, which are basically six planes with rectangular boundaries.
// TODO: still need to return the normal at the intersection// still need to return the normal at the intersection
void Intersector::visit(Box* op, void* ret){

	IsectData* iret = (IsectData*) ret; 

	//if(true) {iret->hit=false; return;}

	Pt3 np = _ray.p*op->getInverseMat(); 
	Pt3 ppp = np*(1/np[3]); 
	Vec3 vvv = _ray.dir*op->getInverseMat(); 

	Ray ray(ppp,vvv); 

	Plane p(Pt3(.5,.5,0),Vec3(0,0,1,0)); 
	double d = GeometryUtils::planeRay(p,ray); 
	Pt3 pp = ray.at(d); 

	iret->hit = false; 
	iret->t0 = FINF32; 

	if(pp[0]>=0&&pp[0]<=1&&pp[1]>=0&&pp[1]<=1){
		if (d>0.001 && iret->t0 > d){
			iret->t0 = d;
			iret->hit = true; 
			iret->normal = Vec3(0,0,-1,0) * transpose(op->getInverseMat());
			iret->normal.normalize();
		}
	}

	p = Plane(Pt3(.5,.5,1),Vec3(0,0,-1,0)); 
	d = GeometryUtils::planeRay(p,ray); 
	pp = ray.at(d); 

	if(pp[0]>=0&&pp[0]<=1&&pp[1]>=0&&pp[1]<=1){
		if (d>0.001 && iret->t0 > d){
			iret->t0 = d;
			iret->hit = true; 
			iret->normal = Vec3(0,0,1,0) * transpose(op->getInverseMat());
			iret->normal.normalize();
		}
	}

	p = Plane(Pt3(.5,0,.5),Vec3(0,1,0,0)); 
	d = GeometryUtils::planeRay(p,ray); 
	pp = ray.at(d); 

	if(pp[0]>=0&&pp[0]<=1&&pp[2]>=0&&pp[2]<=1){
		if (d>0.001 && iret->t0 > d){
			iret->t0 = d;
			iret->hit = true; 
			iret->normal = Vec3(0,-1,0,0) * transpose(op->getInverseMat());
			iret->normal.normalize();
		}
	}

	p = Plane(Pt3(.5,1,.5),Vec3(0,-1,0,0)); 
	d = GeometryUtils::planeRay(p,ray); 
	pp = ray.at(d); 

	if(pp[0]>=0&&pp[0]<=1&&pp[2]>=0&&pp[2]<=1){
		if (d>0.001 && iret->t0 > d){
			iret->t0 = d;
			iret->hit = true; 
			iret->normal = Vec3(0,1,0,0) * transpose(op->getInverseMat());
			iret->normal.normalize();
		}
	}

	p = Plane(Pt3(0,.5,.5),Vec3(1,0,0,0)); 
	d = GeometryUtils::planeRay(p,ray); 
	pp = ray.at(d); 

	if(pp[1]>=0&&pp[1]<=1&&pp[2]>=0&&pp[2]<=1){
		if (d>0.001 && iret->t0 > d){
			iret->t0 = d;
			iret->hit = true; 
			iret->normal = Vec3(-1,0,0,0) * transpose(op->getInverseMat());
			iret->normal.normalize();
		}
	}

	p = Plane(Pt3(1,.5,.5),Vec3(-1,0,0,0)); 
	d = GeometryUtils::planeRay(p,ray); 
	pp = ray.at(d); 

	if(pp[1]>=0&&pp[1]<=1&&pp[2]>=0&&pp[2]<=1){
		if (d>0.001 && iret->t0 > d){
			iret->t0 = d;
			iret->hit = true; 
			iret->normal = Vec3(1,0,0,0) * transpose(op->getInverseMat());
			iret->normal.normalize();
		}
	}
}

/**
 * @brief update the transform matrix of ellipsoid
 * @details [long description]
 */
void Ellipsoid::updateTransform() {
	_mat = compose(_axis1 * _len1, _axis2 * _len2, _axis3 * _len3, _center);
	_imat = !_mat;
	Geometry::updateTransform();
}
void Intersector::visit(Ellipsoid* op, void* ret){
	// TODO: rewrite this as described in the textbook
	IsectData* iret = (IsectData*) ret; 

	// Intersect transformed line with canonical sphere
	// Note that the vector of the trasformed line is normalized
	Pt3 np = _ray.p*op->getInverseMat(); 
	Pt3 ppp = np*(1/np[3]); 
	Vec3 vvv = _ray.dir*op->getInverseMat(); 
	double sizev = mag(vvv);
	vvv.normalize();
	Ray ray(ppp,vvv); 

	Pt3 center(0,0,0); 
	double closest = GeometryUtils::pointRayClosest(center,ray); 
    Vec3 C2P = ray.at(closest) - center; 
	double d2 = C2P*C2P; 

	if(d2 > 1 + EPS){
        iret->hit = false; 
		iret->t0 = 0; 
		return;
	}
	else{
		double h = sqrt(1 - d2); 
		if (closest+h<0){
			iret->hit = false;
			return;
		} else if (closest-h<0.001){
			iret->t0 = closest+h; 
			iret->hit = true;
		} else {
			iret->t0 = closest-h; 
			iret->hit = true; 
		}
		iret->normal = (ray.at(iret->t0) - center); 
		iret->normal.normalize(); 
	}

	// In reality, the hit point t0 is adjusted for the size of vvv
	iret->t0 /= sizev;
	// In reality, the normal is adjusted to be in the right frame
	iret->normal = iret->normal * transpose(op->getInverseMat());
	iret->normal.normalize();
}

void Cylinder::updateTransform() {
	_mat = compose(_baseAxis1 * _lenAxis1, _baseAxis2 * _lenAxis2, _centerAxis * _height, _baseCenter);
	// debugInfo(_centerAxis.toString());
	// debugInfo(_mat.toString());
	_imat = !_mat;
	Geometry::updateTransform();
}
void Intersector::visit(Cylinder* op, void* ret){
	IsectData* iret = (IsectData*) ret; 
	iret->t0 = FINF32;
	iret->hit = false;
	
	// Transform into canonical frame
	// Note that the vector of the trasformed line is normalized
	Pt3 np = _ray.p*op->getInverseMat(); 
	Pt3 ppp = np*(1/np[3]); 
	Vec3 vvv = _ray.dir*op->getInverseMat(); 
	double sizev = mag(vvv);
	vvv.normalize();
	Ray ray(ppp,vvv); 

	// Fast rejection, check distance between rays
	Ray axis(Pt3(0,0,0), Vec3(0,1,0,0));
	double dist = GeometryUtils::rayRayDist(axis, ray);
	if (dist >= 1) {
		return;
	}

	Vec3 vcross = cross(axis.dir,ray.dir); 
	double denom = mag(vcross);

	// Intersect with top and bottom
	Plane p = Plane(Pt3(0,0,0),Vec3(0,1,0,0)); 
	double d = GeometryUtils::planeRay(p,ray); 
	double intdist = mag(ray.at(d) - Pt3(0,0,0));
	if (d>0.001 && intdist<1) {
		iret->hit = true;
		iret->normal = Vec3(0,-1,0,0);
		iret->t0 = d;
	}
	p = Plane(Pt3(0,1,0),Vec3(0,1,0,0));
	d = GeometryUtils::planeRay(p,ray); 
	intdist = mag(ray.at(d) - Pt3(0,1,0));
	if (d>0.001 && intdist<1 && d<iret->t0) {
		iret->hit = true;
		iret->normal = Vec3(0,1,0,0);
		iret->t0 = d;
	}

	if (denom != 0){ // if ray is not parallel to the axis, also check side surface
		// project into xy plane, and intersect with circle
		Mat4 projection = compose(Vec3(1,0,0,0), Vec3(0,0,0,0), Vec3(0,0,1,0), Pt3(0,1,0));
		Pt3 pppp = ppp*projection;
		Vec3 vvvv = vvv*projection;
		Ray planeRay(pppp,vvvv);
		double tclosest = GeometryUtils::pointRayClosest(Pt3(0,0,0), planeRay);
		double h = sqrt(1-dist*dist); // distance
		h /= mag(vvvv); // parameter along ray
		
		double param = tclosest-h;
		if (param > 0.001 && param < iret->t0){
			double zhit = ray.at(param)[1];
			if (zhit>=0 && zhit <= 1){
				iret->hit = true;
				iret->normal = planeRay.at(param) - Pt3(0,0,0);
				iret->t0 = param;
			}
		}
		param = tclosest + h;
		if (param > 0.001 && param < iret->t0){
			double zhit = ray.at(param)[1];
			if (zhit>=0 && zhit <= 1){
				iret->hit = true;
				iret->normal = planeRay.at(param) - Pt3(0,0,0);;
				iret->t0 = param;
			}
		}

	}

	
	// In reality, the hit point t0 is adjusted for the size of vvv
	iret->t0 /= sizev;
	// In reality, the normal is adjusted to be in the right frame
	iret->normal = iret->normal * transpose(op->getInverseMat());
	iret->normal.normalize();

}

void Cone::updateTransform() {
	_mat = compose(_baseAxis1 * _lenAxis1, _baseAxis2 * _lenAxis2, _centerAxis * _height, _baseCenter);
	_imat = !_mat;
	Geometry::updateTransform();
}
void Intersector::visit(Cone* op, void* ret){
	IsectData* iret = (IsectData*) ret; 
	iret->t0 = FINF32;
	iret->hit = false;
	
	// Transform into canonical frame
	// Note that the vector of the trasformed line is normalized
	Pt3 np = _ray.p*op->getInverseMat(); 
	Pt3 ppp = np*(1/np[3]); 
	Vec3 vvv = _ray.dir*op->getInverseMat(); 
	double sizev = mag(vvv);
	vvv.normalize();
	Ray ray(ppp,vvv); 

	// Intersecting against the circle
	Plane p = Plane(Pt3(0,0,0),Vec3(0,1,0,0)); 
	double d = GeometryUtils::planeRay(p,ray); 
	double intdist = mag(ray.at(d) - Pt3(0,0,0));
	if (d>0.001 && intdist<1) {
		iret->hit = true;
		iret->normal = Vec3(0,-1,0,0);
		iret->t0 = d;
	}

	// Intersecting with the cone
	Mat4 projection = compose(Vec3(1,0,0,0), Vec3(0,0,0,0), Vec3(0,0,1,0), Pt3(0,0,0));
	projection[2][3] = -1;
	Pt3 mp1 = ppp * projection;
	Pt3 mp2 = vvv * projection;
	if (mp1[3] == 0 && mp2[3] == 0){
		
	}


	// In reality, the hit point t0 is adjusted for the size of vvv
	iret->t0 /= sizev;
	// In reality, the normal is adjusted to be in the right frame
	iret->normal = iret->normal * transpose(op->getInverseMat());
	iret->normal.normalize();
}

// The operator is the widget that allows you to translate and rotate a geometric object
// It is colored as red/green/blue.  When one of the axis is highlighted, it is yellow.
void Intersector::visit(Operator* op, void* ret){
	IsectAxisData* iret = (IsectAxisData*) ret; 
	Pt3 center = op->getPrimaryOp()->getCenter(); 

	iret->hit = false; 

	if(op->getState()==OP_TRANSLATE){
		Ray rx(center,op->getDirX()); 
		Ray ry(center,op->getDirY()); 
		Ray rz(center,op->getDirZ()); 

		double dx = GeometryUtils::pointRayDist(rx.p+rx.dir*.5,_ray); 
		double dy = GeometryUtils::pointRayDist(ry.p+ry.dir*.5,_ray); 
		double dz = GeometryUtils::pointRayDist(rz.p+rz.dir*.5,_ray); 

		double tol = .12f; 
		if(dx>tol && dy>tol && dz>tol) 
			iret->hit = false; 
		else{
			iret->hit = true; 
			if(dx<dy){
				if(dx<dz) iret->axis = OP_XAXIS; 
				else      iret->axis = OP_ZAXIS; 
			}
			else{
				if(dy<dz) iret->axis = OP_YAXIS; 
				else      iret->axis = OP_ZAXIS; 
			}
		}
	}
	else if(op->getState()==OP_ROTATE){
		Plane xy(center,Vec3(0,0,1,0)); 
		Plane xz(center,Vec3(0,1,0,0)); 
		Plane yz(center,Vec3(1,0,0,0)); 

		double dxy = GeometryUtils::planeRay(xy,_ray); 
		double dxz = GeometryUtils::planeRay(xz,_ray); 
		double dyz = GeometryUtils::planeRay(yz,_ray); 

		double txy = abs(mag(_ray.at(dxy)-center)-OP_STEP); 
		double txz = abs(mag(_ray.at(dxz)-center)-OP_STEP); 
		double tyz = abs(mag(_ray.at(dyz)-center)-OP_STEP); 

		double tol = .05f; 
		if(txy>tol && txz>tol && tyz>tol) 
			iret->hit = false; 
		else{
			iret->hit = true; 
			if(txy<txz){
				if(txy<tyz) iret->axis = OP_ZAXIS; 
				else      iret->axis = OP_XAXIS; 
			}
			else{
				if(txz<tyz) iret->axis = OP_YAXIS; 
				else      iret->axis = OP_XAXIS; 
			}
		}
	}
}
