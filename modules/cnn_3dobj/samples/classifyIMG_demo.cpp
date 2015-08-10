/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <opencv2/cnn_3dobj.hpp>
#include <iomanip>
using namespace cv;
using namespace std;
using namespace cv::cnn_3dobj;
int main(int argc, char** argv)
{
	const String keys = "{help | | this demo will convert a set of images in a particular path into leveldb database for feature extraction using Caffe.}"
		     "{src_dir | ../data/images_all/ | Source direction of the images ready for being used for extract feature as gallery.}"
		     "{caffemodel | ../data/3d_triplet_iter_20000.caffemodel | caffe model for feature exrtaction.}"
		     "{network_forIMG | ../data/3d_triplet_testIMG.prototxt | Network definition file used for extracting feature from a single image and making a classification}"
		     "{mean_file | ../data/images_mean/triplet_mean.binaryproto | The mean file generated by Caffe from all gallery images, this could be used for mean value substraction from all images.}"
		     "{target_img | ../data/images_all/3_13.png | Path of image waiting to be classified.}"
		     "{feature_blob | feat | Name of layer which will represent as the feature, in this network, ip1 or feat is well.}"
		     "{num_candidate | 6 | Number of candidates in gallery as the prediction result.}"
		     "{device | CPU | device}"
		     "{dev_id | 0 | dev_id}";
	cv::CommandLineParser parser(argc, argv, keys);
	parser.about("Demo for Sphere View data generation");
	if (parser.has("help"))
	{
	  parser.printMessage();
	  return 0;
	}
	string src_dir = parser.get<string>("src_dir");
	string caffemodel = parser.get<string>("caffemodel");
	string network_forIMG   = parser.get<string>("network_forIMG");
	string mean_file    = parser.get<string>("mean_file");
	string target_img   = parser.get<string>("target_img");
	string feature_blob = parser.get<string>("feature_blob");
	int num_candidate = parser.get<int>("num_candidate");
	string device = parser.get<string>("device");
	int dev_id = parser.get<int>("dev_id");

	cv::cnn_3dobj::Classification classifier;
	classifier.NetSetter(network_forIMG, caffemodel, mean_file, device, dev_id);
	std::vector<string> name_gallery;
	classifier.list_dir(src_dir.c_str(), name_gallery, false);
	classifier.GetLabellist(name_gallery);
	for (unsigned int i = 0; i < name_gallery.size(); i++) {
	  name_gallery[i] = src_dir + name_gallery[i];
	}
	std::vector<cv::Mat> img_gallery;
	cv::Mat feature_reference;
	for (unsigned int i = 0; i < name_gallery.size(); i++) {
	  img_gallery.push_back(cv::imread(name_gallery[i], -1));
	}
	classifier.FeatureExtract(img_gallery, feature_reference, false, feature_blob);

	std::cout << std::endl << "---------- Prediction for "
	    << target_img << " ----------" << std::endl;

	cv::Mat img = cv::imread(target_img, -1);
	// CHECK(!img.empty()) << "Unable to decode image " << target_img;
	std::cout << std::endl << "---------- Featrue of gallery images ----------" << std::endl;
	std::vector<std::pair<string, float> > prediction;
	for (unsigned int i = 0; i < feature_reference.rows; i++)
	  std::cout << feature_reference.row(i) << endl;
	cv::Mat feature_test;
	classifier.FeatureExtract(img, feature_test, false, feature_blob);
	std::cout << std::endl << "---------- Featrue of target image: " << target_img << "----------" << endl << feature_test << std::endl;
	prediction = classifier.Classify(feature_reference, feature_test, num_candidate);
	// Print the top N prediction.
	std::cout << std::endl << "---------- Prediction result(Distance - File Name in Gallery) ----------" << std::endl;
	for (size_t i = 0; i < prediction.size(); ++i) {
	  std::pair<string, float> p = prediction[i];
	  std::cout << std::fixed << std::setprecision(2) << p.second << " - \""
	        << p.first << "\"" << std::endl;
	}
	return 0;
}
