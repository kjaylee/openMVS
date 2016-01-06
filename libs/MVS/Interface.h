#ifndef _INTERFACE_H_
#define _INTERFACE_H_


// I N C L U D E S /////////////////////////////////////////////////


// D E F I N E S ///////////////////////////////////////////////////

// uncomment to enable BOOST serialization support
// (should be uncommented)
#ifndef _USE_BOOST
//#define _USE_BOOST
#endif

// uncomment to enable BOOST serialization type specializations
// (should be uncommented if serialization specialization not previously implemented)
#ifndef _USE_BOOST_SERIALIZATION
//#defined _USE_BOOST_SERIALIZATION
#endif

// uncomment to enable custom OpenCV data types
// (should be uncommented if OpenCV is not available)
#ifndef _USE_CUSTOM_CV
//#define _USE_CUSTOM_CV
#endif

// uncomment to enable custom serialization code
// (should be uncommented if BOOST is not available)
#if !defined(_USE_BOOST) && !defined(_USE_CUSTOM_ARCHIVE)
#define _USE_CUSTOM_ARCHIVE
#endif


// S T R U C T S ///////////////////////////////////////////////////

#ifdef _USE_CUSTOM_CV

namespace cv {

// simple cv::Matx
template<typename _Tp, int m, int n>
class Matx
{
public:
	_Tp val[m*n];
};

// simple cv::Matx
template<typename _Tp>
class Point3_
{
public:
	_Tp x, y, z;
};

} // namespace cv
#endif
/*----------------------------------------------------------------*/


#if defined(_USE_CUSTOM_ARCHIVE) || !defined(_USE_BOOST)

// custom serialization
#include <fstream>

namespace ARCHIVE {

struct ArchiveSave;
struct ArchiveLoad;

template<typename _Tp>
bool Save(ArchiveSave& a, const _Tp& obj) {
	const_cast<_Tp&>(obj).serialize(a, 0);
	return true;
}
template<typename _Tp>
bool Load(ArchiveLoad& a, _Tp& obj) {
	obj.serialize(a, 0);
	return true;
}


// Basic serialization types
struct ArchiveSave {
	std::ostream& stream;
	ArchiveSave(std::ostream& _stream) : stream(_stream) {}
	template<typename _Tp>
	ArchiveSave& operator & (const _Tp& obj) {
		Save(*this, obj);
		return *this;
	}
};
struct ArchiveLoad {
	std::istream& stream;
	ArchiveLoad(std::istream& _stream) : stream(_stream) {}
	template<typename _Tp>
	ArchiveLoad& operator & (_Tp& obj) {
		Load(*this, obj);
		return *this;
	}
};


// Main exporter & importer
template<typename _Tp>
bool SerializeSave(const _Tp& obj, const String& fileName) {
	std::ofstream stream(fileName, std::ofstream::binary);
	if (!stream.is_open())
		return false;
	ARCHIVE::ArchiveSave serializer(stream);
	serializer & obj;
	return true;
}
template<typename _Tp>
bool SerializeLoad(_Tp& obj, const String& fileName) {
	std::ifstream stream(fileName, std::ifstream::binary);
	if (!stream.is_open())
		return false;
	ARCHIVE::ArchiveLoad serializer(stream);
	serializer & obj;
	return true;
}


#define ARCHIVE_DEFINE_TYPE(TYPE) \
template<> \
bool Save<TYPE>(ArchiveSave& a, const TYPE& v) { \
	a.stream.write((const char*)&v, sizeof(TYPE)); \
	return true; \
} \
template<> \
bool Load<TYPE>(ArchiveLoad& a, TYPE& v) { \
	a.stream.read((char*)&v, sizeof(TYPE)); \
	return true; \
}

// Serialization support for basic types
ARCHIVE_DEFINE_TYPE(uint32_t)
ARCHIVE_DEFINE_TYPE(uint64_t)
ARCHIVE_DEFINE_TYPE(float)
ARCHIVE_DEFINE_TYPE(double)

// Serialization support for cv::Matx
template<typename _Tp, int m, int n>
bool Save(ArchiveSave& a, const cv::Matx<_Tp,m,n>& _m) {
	a.stream.write((const char*)_m.val, sizeof(_Tp)*m*n);
	return true;
}
template<typename _Tp, int m, int n>
bool Load(ArchiveLoad& a, cv::Matx<_Tp,m,n>& _m) {
	a.stream.read((char*)_m.val, sizeof(_Tp)*m*n);
	return true;
}

// Serialization support for cv::Point3_
template<typename _Tp>
bool Save(ArchiveSave& a, const cv::Point3_<_Tp>& pt) {
	a.stream.write((const char*)&pt.x, sizeof(_Tp)*3);
	return true;
}
template<typename _Tp>
bool Load(ArchiveLoad& a, cv::Point3_<_Tp>& pt) {
	a.stream.read((char*)&pt.x, sizeof(_Tp)*3);
	return true;
}

// Serialization support for std::string
template<>
bool Save<std::string>(ArchiveSave& a, const std::string& s) {
	const size_t size(s.size());
	Save(a, size);
	if (size > 0)
		a.stream.write(&s[0], sizeof(char)*size);
	return true;
}
template<>
bool Load<std::string>(ArchiveLoad& a, std::string& s) {
	size_t size;
	Load(a, size);
	if (size > 0) {
		s.resize(size);
		a.stream.read(&s[0], sizeof(char)*size);
	}
	return true;
}

// Serialization support for std::vector
template<typename _Tp>
bool Save(ArchiveSave& a, const std::vector<_Tp>& v) {
	const size_t size(v.size());
	Save(a, size);
	for (size_t i=0; i<size; ++i)
		Save(a, v[i]);
	return true;
}
template<typename _Tp>
bool Load(ArchiveLoad& a, std::vector<_Tp>& v) {
	size_t size;
	Load(a, size);
	if (size > 0) {
		v.resize(size);
		for (size_t i=0; i<size; ++i)
			Load(a, v[i]);
	}
	return true;
}
} // namespace ARCHIVE

#elif defined(_USE_BOOST_SERIALIZATION)

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>

namespace boost {
namespace serialization {

// Serialization support for cv::Matx
template<class Archive, typename _Tp, int m, int n>
void serialize(Archive& ar, cv::Matx<_Tp,m,n>& _m, const unsigned int /*version*/) {
	ar & _m.val;
}

// Serialization support for cv::Point3_
template<class Archive, typename _Tp>
void serialize(Archive& ar, cv::Point3_<_Tp>& pt, const unsigned int /*version*/) {
	ar & pt.x & pt.y & pt.z;
}

} // namespace serialization
} // namespace boost

#endif
/*----------------------------------------------------------------*/


namespace MVS {

// interface used to export/import MVS input data;
// MAX(width,height) is used for normalization
struct Interface
{
	typedef float Real;
	typedef cv::Point3_<Real> Pos3;
	typedef cv::Matx<Real,3,3> Mat33;
	typedef uint8_t Color;
	typedef cv::Point3_<Color> Col3; // x=B, y=G, z=R
	/*----------------------------------------------------------------*/

	// structure describing a mobile platform with cameras attached to it
	struct Platform {
		// structure describing a camera mounted on a platform
		struct Camera {
			std::string name; // camera's name
			Mat33 K; // camera's normalized intrinsics matrix
			Mat33 R; // camera's rotation matrix relative to the platform
			Pos3 C; // camera's translation vector relative to the platform

			template <class Archive>
			void serialize(Archive& ar, const unsigned int /*version*/) {
				ar & name;
				ar & K;
				ar & R;
				ar & C;
			}
		};
		typedef std::vector<Camera> CameraArr;

		// structure describing a pose along the trajectory of a platform
		struct Pose {
			Mat33 R; // platform's rotation matrix
			Pos3 C; // platform's translation vector in the global coordinate system

			template <class Archive>
			void serialize(Archive& ar, const unsigned int /*version*/) {
				ar & R;
				ar & C;
			}
		};
		typedef std::vector<Pose> PoseArr;

		std::string name; // platform's name
		CameraArr cameras; // cameras mounted on the platform
		PoseArr poses; // trajectory of the platform

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & name;
			ar & cameras;
			ar & poses;
		}
	};
	typedef std::vector<Platform> PlatformArr;
	/*----------------------------------------------------------------*/

	// structure describing an image
	struct Image {
		std::string name; // image file name
		uint32_t platformID; // ID of the associated platform
		uint32_t cameraID; // ID of the associated camera on the associated platform
		uint32_t poseID; // ID of the pose of the associated platform

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & name;
			ar & platformID;
			ar & cameraID;
			ar & poseID;
		}
	};
	typedef std::vector<Image> ImageArr;
	/*----------------------------------------------------------------*/

	// structure describing a 3D point
	struct Vertex {
		// structure describing one view for a given 3D feature
		struct View {
			uint32_t imageID; // image ID corresponding to this view
			Real confidence; // view's confidence (0 - not available)

			template<class Archive>
			void serialize(Archive& ar, const unsigned int /*version*/) {
				ar & imageID;
				ar & confidence;
			}
		};
		typedef std::vector<View> ViewArr;

		Pos3 X; // 3D point position
		ViewArr views; // list of all available views for this 3D feature

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & X;
			ar & views;
		}
	};
	typedef std::vector<Vertex> VertexArr;
	/*----------------------------------------------------------------*/

	// structure describing a 3D point's normal (optional)
	struct VertexNormal {
		Pos3 n; // 3D feature normal

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & n;
		}
	};
	typedef std::vector<VertexNormal> VertexNormalArr;
	/*----------------------------------------------------------------*/

	// structure describing a 3D point's color (optional)
	struct VertexColor {
		Col3 c; // 3D feature color

		template <class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			ar & c;
		}
	};
	typedef std::vector<VertexColor> VertexColorArr;
	/*----------------------------------------------------------------*/

	PlatformArr platforms; // array of platforms
	ImageArr images; // array of images
	VertexArr vertices; // array of reconstructed 3D points
	VertexNormalArr verticesNormal; // array of reconstructed 3D points' normal (optional)
	VertexColorArr verticesColor; // array of reconstructed 3D points' color (optional)

	template <class Archive>
	void serialize(Archive& ar, const unsigned int /*version*/) {
		ar & platforms;
		ar & images;
		ar & vertices;
		ar & verticesNormal;
		ar & verticesColor;
	}
};
/*----------------------------------------------------------------*/

} // namespace MVS

#endif // _INTERFACE_H_

