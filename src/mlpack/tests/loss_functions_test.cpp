/**
 * @file tests/loss_functions_test.cpp
 * @author Dakshit Agrawal
 * @author Sourabh Varshney
 * @author Atharva Khandait
 * @author Saksham Rastogi
 *
 * Tests for loss functions in mlpack::methods::ann:loss_functions.
 *
 * mlpack is free software; you may redistribute it and/or modify it under the
 * terms of the 3-clause BSD license.  You should have received a copy of the
 * 3-clause BSD license along with mlpack.  If not, see
 * http://www.opensource.org/licenses/BSD-3-Clause for more information.
 */
#include <mlpack/core.hpp>

#include <mlpack/methods/ann/layer/layer.hpp>
#include <mlpack/methods/ann/loss_functions/huber_loss.hpp>
#include <mlpack/methods/ann/loss_functions/poisson_nll_loss.hpp>
#include <mlpack/methods/ann/loss_functions/kl_divergence.hpp>
#include <mlpack/methods/ann/loss_functions/earth_mover_distance.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <mlpack/methods/ann/loss_functions/sigmoid_cross_entropy_error.hpp>
#include <mlpack/methods/ann/loss_functions/cross_entropy_error.hpp>
#include <mlpack/methods/ann/loss_functions/reconstruction_loss.hpp>
#include <mlpack/methods/ann/loss_functions/margin_ranking_loss.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_logarithmic_error.hpp>
#include <mlpack/methods/ann/loss_functions/mean_bias_error.hpp>
#include <mlpack/methods/ann/loss_functions/dice_loss.hpp>
#include <mlpack/methods/ann/loss_functions/log_cosh_loss.hpp>
#include <mlpack/methods/ann/loss_functions/hinge_embedding_loss.hpp>
#include <mlpack/methods/ann/loss_functions/cosine_embedding_loss.hpp>
#include <mlpack/methods/ann/loss_functions/l1_loss.hpp>
#include <mlpack/methods/ann/loss_functions/soft_margin_loss.hpp>
#include <mlpack/methods/ann/loss_functions/mean_absolute_percentage_error.hpp>
#include <mlpack/methods/ann/init_rules/nguyen_widrow_init.hpp>
#include <mlpack/methods/ann/ffn.hpp>

#include <boost/test/unit_test.hpp>
#include "test_tools.hpp"
#include "ann_test_tools.hpp"

using namespace mlpack;
using namespace mlpack::ann;

BOOST_AUTO_TEST_SUITE(LossFunctionsTest);

/**
 * Simple Huber Loss test.
 */
BOOST_AUTO_TEST_CASE(HuberLossTest)
{
  arma::mat input, target, output;
  HuberLoss<> module;

  // Test the Forward function.
  input = arma::mat("17.45 12.91 13.63 29.01 7.12 15.47 31.52 31.97");
  target = arma::mat("16.52 13.11 13.67 29.51 24.31 15.03 30.72 34.07");
  double loss = module.Forward(input, target);
  BOOST_REQUIRE_CLOSE_FRACTION(loss, 2.410631, 0.00001);

  // Test the backward function.
  module.Backward(input, target, output);

  // Expected Output:
  // [0.1162 -0.0250 -0.0050 -0.0625 -0.1250  0.0550  0.1000 -0.1250]
  // Sum of Expected Output = -0.07125.
  double expectedOutputSum = arma::accu(output);
  BOOST_REQUIRE_CLOSE_FRACTION(expectedOutputSum, -0.07125, 0.00001);

  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);
}

/**
 * Poisson Negative Log Likelihood Loss function test.
 */
BOOST_AUTO_TEST_CASE(PoissonNLLLossTest)
{
  arma::mat input, target, input4, target4;
  arma::mat output1, output2, output3, output4;
  arma::mat expOutput1, expOutput2, expOutput3, expOutput4;
  PoissonNLLLoss<> module1;
  PoissonNLLLoss<> module2(true, true, 1e-08, false);
  PoissonNLLLoss<> module3(true, true, 1e-08, true);
  PoissonNLLLoss<> module4(false, true, 1e-08, true);

  // Test the Forward function on a user generated input.
  input = arma::mat("1.0 1.0 1.9 1.6 -1.9 3.7 -1.0 0.5");
  target = arma::mat("1.0 3.0 1.0 2.0 1.0 4.0 2.0 1.0");

  // Input required for module 4. Probs are in range [0, 1].
  input4 = arma::mat("0.658502 0.445627 0.667651 0.310549 \
                      0.589540 0.052568 0.549769 0.381504 ");
  target4 = arma::mat("1.0 3.0 1.0 2.0 1.0 4.0 2.0 1.0");

  double loss1 = module1.Forward(input, target);
  double loss2 = module2.Forward(input, target);
  double loss3 = module3.Forward(input, target);
  double loss4 = module4.Forward(input4, target4);
  BOOST_REQUIRE_CLOSE_FRACTION(loss1, 4.8986, 0.0001);
  BOOST_REQUIRE_CLOSE_FRACTION(loss2, 45.4139, 0.0001);
  BOOST_REQUIRE_CLOSE_FRACTION(loss3, 5.6767, 0.0001);
  BOOST_REQUIRE_CLOSE_FRACTION(loss4, 3.742157, 0.0001);

  // Test the Backward function.
  module1.Backward(input, target, output1);
  module2.Backward(input, target, output2);
  module3.Backward(input, target, output3);
  module4.Backward(input4, target4, output4);

  expOutput1 = arma::mat("0.214785 -0.0352148 0.710737 0.369129 \
                         -0.106304 4.55591 -0.204015 0.0810902");
  expOutput2 = arma::mat("1.71828 -0.281718 5.68589 2.95303\
                         -0.850431 36.4473 -1.63212 0.648721");
  expOutput3 = arma::mat("0.214785 -0.035215 0.710737 0.369129 \
                         -0.106304 4.555913 -0.204015 0.081090");
  expOutput4 = arma::mat("-0.064825 -0.716511 -0.062224 -0.680027 \
                          -0.087030 -9.386517 -0.329736 -0.202650");

  BOOST_REQUIRE_EQUAL(output1.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output1.n_cols, input.n_cols);

  BOOST_REQUIRE_EQUAL(output2.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output2.n_cols, input.n_cols);

  BOOST_REQUIRE_EQUAL(output3.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output3.n_cols, input.n_cols);

  BOOST_REQUIRE_EQUAL(output4.n_rows, input4.n_rows);
  BOOST_REQUIRE_EQUAL(output4.n_cols, input4.n_cols);

  for (size_t i = 0; i < expOutput1.n_elem; ++i)
  {
    BOOST_REQUIRE_CLOSE_FRACTION(output1[i], expOutput1[i], 0.0001);
    BOOST_REQUIRE_CLOSE_FRACTION(output2[i], expOutput2[i], 0.0001);
    BOOST_REQUIRE_CLOSE_FRACTION(output3[i], expOutput3[i], 0.0001);
    BOOST_REQUIRE_CLOSE_FRACTION(output4[i], expOutput4[i], 0.0001);
  }
}

/**
 * Simple KL Divergence test.  The loss should be zero if input = target.
 */
BOOST_AUTO_TEST_CASE(SimpleKLDivergenceTest)
{
  arma::mat input, target, output;
  double loss;
  KLDivergence<> module(true);

  // Test the Forward function.  Loss should be 0 if input = target.
  input = arma::ones(10, 1);
  target = arma::ones(10, 1);
  loss = module.Forward(input, target);
  BOOST_REQUIRE_SMALL(loss, 0.00001);
}

/*
 * Simple test for the mean squared logarithmic error function.
 */
BOOST_AUTO_TEST_CASE(SimpleMeanSquaredLogarithmicErrorTest)
{
  arma::mat input, output, target;
  MeanSquaredLogarithmicError<> module;

  // Test the Forward function on a user generator input and compare it against
  // the manually calculated result.
  input = arma::zeros(1, 8);
  target = arma::zeros(1, 8);
  double error = module.Forward(input, target);
  BOOST_REQUIRE_SMALL(error, 0.00001);

  // Test the Backward function.
  module.Backward(input, target, output);
  // The output should be equal to 0.
  CheckMatrices(input, output);
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);

  // Test the error function on a single input.
  input = arma::mat("2");
  target = arma::mat("3");
  error = module.Forward(input, target);
  BOOST_REQUIRE_CLOSE(error, 0.082760974810151655, 0.001);

  // Test the Backward function on a single input.
  module.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE(arma::accu(output), -0.1917880483011872, 0.001);
  BOOST_REQUIRE_EQUAL(output.n_elem, 1);
}

/**
 * Test to check KL Divergence loss function when we take mean.
 */
BOOST_AUTO_TEST_CASE(KLDivergenceMeanTest)
{
  arma::mat input, target, output;
  double loss;
  KLDivergence<> module(true);

  // Test the Forward function.
  input = arma::mat("1 1 1 1 1 1 1 1 1 1");
  target = arma::exp(arma::mat("2 1 1 1 1 1 1 1 1 1"));

  loss = module.Forward(input, target);
  BOOST_REQUIRE_CLOSE_FRACTION(loss, -1.1 , 0.00001);

  // Test the Backward function.
  module.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE_FRACTION(arma::as_scalar(output), -0.1, 0.00001);
}

/**
 * Test to check KL Divergence loss function when we do not take mean.
 */
BOOST_AUTO_TEST_CASE(KLDivergenceNoMeanTest)
{
  arma::mat input, target, output;
  double loss;
  KLDivergence<> module(false);

  // Test the Forward function.
  input = arma::mat("1 1 1 1 1 1 1 1 1 1");
  target = arma::exp(arma::mat("2 1 1 1 1 1 1 1 1 1"));

  loss = module.Forward(input, target);
  BOOST_REQUIRE_CLOSE_FRACTION(loss, -11, 0.00001);

  // Test the Backward function.
  module.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE_FRACTION(arma::as_scalar(output), -1, 0.00001);
}

/*
 * Simple test for the mean squared error performance function.
 */
BOOST_AUTO_TEST_CASE(SimpleMeanSquaredErrorTest)
{
  arma::mat input, output, target;
  MeanSquaredError<> module;

  // Test the Forward function on a user generator input and compare it against
  // the manually calculated result.
  input = arma::mat("1.0 0.0 1.0 0.0 -1.0 0.0 -1.0 0.0");
  target = arma::zeros(1, 8);
  double error = module.Forward(input, target);
  BOOST_REQUIRE_EQUAL(error, 0.5);

  // Test the Backward function.
  module.Backward(input, target, output);
  // We subtract a zero vector, so according to the used backward formula:
  // output = 2 * (input - target) / target.n_cols,
  // output * nofColumns / 2 should be equal to input.
  CheckMatrices(input, output * output.n_cols / 2);
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);

  // Test the error function on a single input.
  input = arma::mat("2");
  target = arma::mat("3");
  error = module.Forward(input, target);
  BOOST_REQUIRE_EQUAL(error, 1.0);

  // Test the Backward function on a single input.
  module.Backward(input, target, output);
  // Test whether the output is negative.
  BOOST_REQUIRE_EQUAL(arma::accu(output), -2);
  BOOST_REQUIRE_EQUAL(output.n_elem, 1);
}

/*
 * Simple test for the cross-entropy error performance function.
 */
BOOST_AUTO_TEST_CASE(SimpleCrossEntropyErrorTest)
{
  arma::mat input1, input2, output, target1, target2;
  CrossEntropyError<> module(1e-6);

  // Test the Forward function on a user generator input and compare it against
  // the manually calculated result.
  input1 = arma::mat("0.5 0.5 0.5 0.5 0.5 0.5 0.5 0.5");
  target1 = arma::zeros(1, 8);
  double error1 = module.Forward(input1, target1);
  BOOST_REQUIRE_SMALL(error1 - 8 * std::log(2), 2e-5);

  input2 = arma::mat("0 1 1 0 1 0 0 1");
  target2 = arma::mat("0 1 1 0 1 0 0 1");
  double error2 = module.Forward(input2, target2);
  BOOST_REQUIRE_SMALL(error2, 1e-5);

  // Test the Backward function.
  module.Backward(input1, target1, output);
  for (double el : output)
  {
    // For the 0.5 constant vector we should get 1 / (1 - 0.5) = 2 everywhere.
    BOOST_REQUIRE_SMALL(el - 2, 5e-6);
  }
  BOOST_REQUIRE_EQUAL(output.n_rows, input1.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input1.n_cols);

  module.Backward(input2, target2, output);
  for (size_t i = 0; i < 8; ++i)
  {
    double el = output.at(0, i);
    if (input2.at(i) == 0)
      BOOST_REQUIRE_SMALL(el - 1, 2e-6);
    else
      BOOST_REQUIRE_SMALL(el + 1, 2e-6);
  }
  BOOST_REQUIRE_EQUAL(output.n_rows, input2.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input2.n_cols);
}

/**
 * Simple test for the Sigmoid Cross Entropy performance function.
 */
BOOST_AUTO_TEST_CASE(SimpleSigmoidCrossEntropyErrorTest)
{
  arma::mat input1, input2, input3, output, target1,
            target2, target3, expectedOutput;
  SigmoidCrossEntropyError<> module;

  // Test the Forward function on a user generator input and compare it against
  // the calculated result.
  input1 = arma::mat("0.5 0.5 0.5 0.5 0.5 0.5 0.5 0.5");
  target1 = arma::zeros(1, 8);
  double error1 = module.Forward(input1, target1);
  double expected = 0.97407699;
  // Value computed using tensorflow.
  BOOST_REQUIRE_SMALL(error1 / input1.n_elem - expected, 1e-7);

  input2 = arma::mat("1 2 3 4 5");
  target2 = arma::mat("0 0 1 0 1");
  double error2 = module.Forward(input2, target2);
  expected = 1.5027283;
  BOOST_REQUIRE_SMALL(error2 / input2.n_elem - expected, 1e-6);

  input3 = arma::mat("0 -1 -1 0 -1 0 0 -1");
  target3 = arma::mat("0 -1 -1 0 -1 0 0 -1");
  double error3 = module.Forward(input3, target3);
  expected = 0.00320443;
  BOOST_REQUIRE_SMALL(error3 / input3.n_elem - expected, 1e-6);

  // Test the Backward function.
  module.Backward(input1, target1, output);
  expected = 0.62245929;
  for (size_t i = 0; i < output.n_elem; ++i)
    BOOST_REQUIRE_SMALL(output(i) - expected, 1e-5);
  BOOST_REQUIRE_EQUAL(output.n_rows, input1.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input1.n_cols);

  expectedOutput = arma::mat(
      "0.7310586 0.88079709 -0.04742587 0.98201376 -0.00669285");
  module.Backward(input2, target2, output);
  for (size_t i = 0; i < output.n_elem; ++i)
    BOOST_REQUIRE_SMALL(output(i) - expectedOutput(i), 1e-5);
  BOOST_REQUIRE_EQUAL(output.n_rows, input2.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input2.n_cols);

  module.Backward(input3, target3, output);
  expectedOutput = arma::mat("0.5 1.2689414");
  for (size_t i = 0; i < 8; ++i)
  {
    double el = output.at(0, i);
    if (std::abs(input3.at(i) - 0.0) < 1e-5)
      BOOST_REQUIRE_SMALL(el - expectedOutput[0], 2e-6);
    else
      BOOST_REQUIRE_SMALL(el - expectedOutput[1], 2e-6);
  }
  BOOST_REQUIRE_EQUAL(output.n_rows, input3.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input3.n_cols);
}

/**
 * Simple test for the Earth Mover Distance Layer.
 */
BOOST_AUTO_TEST_CASE(SimpleEarthMoverDistanceLayerTest)
{
  arma::mat input1, input2, output, target1, target2, expectedOutput;
  EarthMoverDistance<> module;

  // Test the Forward function on a user generator input and compare it against
  // the manually calculated result.
  input1 = arma::mat("0.5 0.5 0.5 0.5 0.5 0.5 0.5 0.5");
  target1 = arma::zeros(1, 8);
  double error1 = module.Forward(input1, target1);
  double expected = 0.0;
  BOOST_REQUIRE_SMALL(error1 / input1.n_elem - expected, 1e-7);

  input2 = arma::mat("1 2 3 4 5");
  target2 = arma::mat("1 0 1 0 1");
  double error2 = module.Forward(input2, target2);
  expected = -1.8;
  BOOST_REQUIRE_SMALL(error2 / input2.n_elem - expected, 1e-6);

  // Test the Backward function.
  module.Backward(input1, target1, output);
  expected = 0.0;
  for (size_t i = 0; i < output.n_elem; ++i)
    BOOST_REQUIRE_SMALL(output(i) - expected, 1e-5);
  BOOST_REQUIRE_EQUAL(output.n_rows, input1.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input1.n_cols);

  expectedOutput = arma::mat("-1 0 -1 0 -1");
  module.Backward(input2, target2, output);
  for (size_t i = 0; i < output.n_elem; ++i)
    BOOST_REQUIRE_SMALL(output(i) - expectedOutput(i), 1e-5);
  BOOST_REQUIRE_EQUAL(output.n_rows, input2.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input2.n_cols);
}

/*
 * Mean Squared Error numerical gradient test.
 */
BOOST_AUTO_TEST_CASE(GradientMeanSquaredErrorTest)
{
  // Linear function gradient instantiation.
  struct GradientFunction
  {
    GradientFunction()
    {
      input = arma::randu(10, 1);
      target = arma::randu(2, 1);

      model = new FFN<MeanSquaredError<>, NguyenWidrowInitialization>();
      model->Predictors() = input;
      model->Responses() = target;
      model->Add<IdentityLayer<> >();
      model->Add<Linear<> >(10, 2);
      model->Add<SigmoidLayer<> >();
    }

    ~GradientFunction()
    {
      delete model;
    }

    double Gradient(arma::mat& gradient) const
    {
      arma::mat output;
      double error = model->Evaluate(model->Parameters(), 0, 1);
      model->Gradient(model->Parameters(), 0, gradient, 1);
      return error;
    }

    arma::mat& Parameters() { return model->Parameters(); }

    FFN<MeanSquaredError<>, NguyenWidrowInitialization>* model;
    arma::mat input, target;
  } function;

  BOOST_REQUIRE_LE(CheckGradient(function), 1e-4);
}

/*
 * Reconstruction Loss numerical gradient test.
 */
BOOST_AUTO_TEST_CASE(GradientReconstructionLossTest)
{
  // Linear function gradient instantiation.
  struct GradientFunction
  {
    GradientFunction()
    {
      input = arma::randu(10, 1);
      target = arma::randu(2, 1);

      model = new FFN<ReconstructionLoss<>, NguyenWidrowInitialization>();
      model->Predictors() = input;
      model->Responses() = target;
      model->Add<IdentityLayer<> >();
      model->Add<Linear<> >(10, 2);
      model->Add<SigmoidLayer<> >();
    }

    ~GradientFunction()
    {
      delete model;
    }

    double Gradient(arma::mat& gradient) const
    {
      arma::mat output;
      double error = model->Evaluate(model->Parameters(), 0, 1);
      model->Gradient(model->Parameters(), 0, gradient, 1);
      return error;
    }

    arma::mat& Parameters() { return model->Parameters(); }

    FFN<ReconstructionLoss<>, NguyenWidrowInitialization>* model;
    arma::mat input, target;
  } function;

  BOOST_REQUIRE_LE(CheckGradient(function), 1e-4);
}

/*
 * Simple test for the dice loss function.
 */
BOOST_AUTO_TEST_CASE(DiceLossTest)
{
  arma::mat input1, input2, target, output;
  double loss;
  DiceLoss<> module;

  // Test the Forward function. Loss should be 0 if input = target.
  input1 = arma::ones(10, 1);
  target = arma::ones(10, 1);
  loss = module.Forward(input1, target);
  BOOST_REQUIRE_SMALL(loss, 0.00001);

  // Test the Forward function. Loss should be 0.185185185.
  input2 = arma::ones(10, 1) * 0.5;
  loss = module.Forward(input2, target);
  BOOST_REQUIRE_CLOSE(loss, 0.185185185, 0.00001);

  // Test the Backward function for input = target.
  module.Backward(input1, target, output);
  for (double el : output)
  {
    // For input = target we should get 0.0 everywhere.
    BOOST_REQUIRE_CLOSE(el, 0.0, 0.00001);
  }
  BOOST_REQUIRE_EQUAL(output.n_rows, input1.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input1.n_cols);

  // Test the Backward function.
  module.Backward(input2, target, output);
  for (double el : output)
  {
    // For the 0.5 constant vector we should get -0.0877914951989026 everywhere.
    BOOST_REQUIRE_CLOSE(el, -0.0877914951989026, 0.00001);
  }
  BOOST_REQUIRE_EQUAL(output.n_rows, input2.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input2.n_cols);
}

/*
 * Simple test for the mean bias error performance function.
 */
BOOST_AUTO_TEST_CASE(SimpleMeanBiasErrorTest)
{
  arma::mat input, output, target;
  MeanBiasError<> module;

  // Test the Forward function on a user generator input and compare it against
  // the manually calculated result.
  input = arma::mat("1.0 0.0 1.0 -1.0 -1.0 0.0 -1.0 0.0");
  target = arma::zeros(1, 8);
  double error = module.Forward(input, target);
  BOOST_REQUIRE_EQUAL(error, 0.125);

  // Test the Backward function.
  module.Backward(input, target, output);
  // We should get a vector with -1 everywhere.
  for (double el : output)
  {
    BOOST_REQUIRE_EQUAL(el, -1);
  }
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);

  // Test the error function on a single input.
  input = arma::mat("2");
  target = arma::mat("3");
  error = module.Forward(input, target);
  BOOST_REQUIRE_EQUAL(error, 1.0);

  // Test the Backward function on a single input.
  module.Backward(input, target, output);
  // Test whether the output is negative.
  BOOST_REQUIRE_EQUAL(arma::accu(output), -1);
  BOOST_REQUIRE_EQUAL(output.n_elem, 1);
}

/**
 * Simple test for the Log-Hyperbolic-Cosine loss function.
 */
BOOST_AUTO_TEST_CASE(LogCoshLossTest)
{
  arma::mat input, target, output;
  double loss;
  LogCoshLoss<> module(2);

  // Test the Forward function. Loss should be 0 if input = target.
  input = arma::ones(10, 1);
  target = arma::ones(10, 1);
  loss = module.Forward(input, target);
  BOOST_REQUIRE_EQUAL(loss, 0);

  // Test the Backward function for input = target.
  module.Backward(input, target, output);
  for (double el : output)
  {
    // For input = target we should get 0.0 everywhere.
    BOOST_REQUIRE_CLOSE(el, 0.0, 1e-5);
  }

  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);

  // Test the Forward function. Loss should be 0.546621.
  input = arma::mat("1 2 3 4 5");
  target = arma::mat("1 2.4 3.4 4.2 5.5");
  loss = module.Forward(input, target);
  BOOST_REQUIRE_CLOSE(loss, 0.546621, 1e-3);

  // Test the Backward function.
  module.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE(arma::accu(output), 2.46962, 1e-3);
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);
}

/**
 * Simple test for the Hinge Embedding loss function.
 */
BOOST_AUTO_TEST_CASE(HingeEmbeddingLossTest)
{
  arma::mat input, target, output;
  double loss;
  HingeEmbeddingLoss<> module;

  // Test the Forward function. Loss should be 0 if input = target.
  input = arma::ones(10, 1);
  target = arma::ones(10, 1);
  loss = module.Forward(input, target);
  BOOST_REQUIRE_EQUAL(loss, 0);

  // Test the Backward function for input = target.
  module.Backward(input, target, output);
  for (double el : output)
  {
    // For input = target we should get 0.0 everywhere.
    BOOST_REQUIRE_CLOSE(el, 0.0, 1e-5);
  }

  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);

  // Test the Forward function. Loss should be 0.84.
  input = arma::mat("0.1 0.8 0.6 0.0 0.5");
  target = arma::mat("0 1.0 1.0 0 0");
  loss = module.Forward(input, target);
  BOOST_REQUIRE_CLOSE(loss, 0.84, 1e-3);

  // Test the Backward function.
  module.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE(arma::accu(output), -2, 1e-3);
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);
}

/**
 * Simple test for the l1 loss function.
 */
BOOST_AUTO_TEST_CASE(SimpleL1LossTest)
{
  arma::mat input1, input2, output, target1, target2;
  L1Loss<> module(false);

  // Test the Forward function on a user generator input and compare it against
  // the manually calculated result.
  input1 = arma::mat("0.5 0.5 0.5 0.5 0.5 0.5 0.5");
  target1 = arma::zeros(1, 7);
  double error1 = module.Forward(input1, target1);
  BOOST_REQUIRE_EQUAL(error1, 3.5);

  input2 = arma::mat("0 1 1 0 1 0 0 1");
  target2 = arma::mat("0 1 1 0 1 0 0 1");
  double error2 = module.Forward(input2, target2);
  BOOST_REQUIRE_CLOSE(error2, 0.0, 0.00001);

  // Test the Backward function.
  module.Backward(input1, target1, output);
  for (double el : output)
    BOOST_REQUIRE_EQUAL(el , 1);

  BOOST_REQUIRE_EQUAL(output.n_rows, input1.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input1.n_cols);

  module.Backward(input2, target2, output);
  for (double el : output)
    BOOST_REQUIRE_EQUAL(el, 0);

  BOOST_REQUIRE_EQUAL(output.n_rows, input2.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input2.n_cols);
}

/**
 * Simple test for the Cosine Embedding loss function.
 */
BOOST_AUTO_TEST_CASE(CosineEmbeddingLossTest)
{
  arma::mat input1, input2, y, output;
  double loss;
  CosineEmbeddingLoss<> module;

  // Test the Forward function. Loss should be 0 if input1 = input2 and y = 1.
  input1 = arma::mat(1, 10);
  input2 = arma::mat(1, 10);
  input1.ones();
  input2.ones();
  y = arma::mat(1, 1);
  y.ones();
  loss = module.Forward(input1, input1);
  BOOST_REQUIRE_SMALL(loss, 1e-6);

  // Test the Backward function.
  module.Backward(input1, input1, output);
  BOOST_REQUIRE_SMALL(arma::accu(output), 1e-6);

  // Check for dissimilarity.
  module.Similarity() = false;
  loss = module.Forward(input1, input1);
  BOOST_REQUIRE_CLOSE(loss, 1.0, 1e-4);

  // Test the Backward function.
  module.Backward(input1, input1, output);
  BOOST_REQUIRE_SMALL(arma::accu(output), 1e-6);

  input1 = arma::mat(3, 2);
  input2 = arma::mat(3, 2);
  input1.fill(1);
  input1(4) = 2;
  input2.fill(1);
  input2(0) = 2;
  input2(1) = 2;
  input2(2) = 2;
  loss = module.Forward(input1, input2);
  // Calculated using torch.nn.CosineEmbeddingLoss().
  BOOST_REQUIRE_CLOSE(loss, 2.897367, 1e-3);

  // Test the Backward function.
  module.Backward(input1, input2, output);
  BOOST_REQUIRE_CLOSE(arma::accu(output), 0.06324556, 1e-3);

  // Check for correctness for cube.
  CosineEmbeddingLoss<> module2(0.5, true);

  arma::cube input3(3, 2, 2);
  arma::cube input4(3, 2, 2);
  input3.fill(1);
  input4.fill(1);
  input3(0) = 2;
  input3(1) = 2;
  input3(4) = 2;
  input3(6) = 2;
  input3(8) = 2;
  input3(10) = 2;
  input4(2) = 2;
  input4(9) = 2;
  input4(11) = 2;
  loss = module2.Forward(input3, input4);
  // Calculated using torch.nn.CosineEmbeddingLoss().
  BOOST_REQUIRE_CLOSE(loss, 0.55395, 1e-3);

  // Test the Backward function.
  module2.Backward(input3, input4, output);
  BOOST_REQUIRE_CLOSE(arma::accu(output), -0.36649111, 1e-3);

  // Check Output for mean type of reduction.
  CosineEmbeddingLoss<> module3(0.0, true, true);
  loss = module3.Forward(input3, input4);
  BOOST_REQUIRE_CLOSE(loss, 0.092325, 1e-3);

  // Check correctness for cube.
  module3.Similarity() = false;
  loss = module3.Forward(input3, input4);
  BOOST_REQUIRE_CLOSE(loss, 0.90767498236, 1e-3);

  // Test the Backward function.
  module3.Backward(input3, input4, output);
  BOOST_REQUIRE_CLOSE(arma::accu(output), 0.36649111, 1e-4);
}

/*
 * Simple test for the Margin Ranking Loss function.
 */
BOOST_AUTO_TEST_CASE(MarginRankingLossTest)
{
  arma::mat input, input1, input2, target, output;
  MarginRankingLoss<> module;

  // Test the Forward function on a user generator input and compare it against
  // the manually calculated result.
  input1 = arma::mat("1 2 5 7 -1 -3");
  input2 = arma::mat("-1 3 -4 11 3 -3");
  input = arma::join_cols(input1, input2);
  target = arma::mat("1 -1 -1 1 -1 1");
  double error = module.Forward(input, target);
  // Computed using torch.nn.functional.margin_ranking_loss()
  BOOST_REQUIRE_CLOSE(error, 2.66667, 1e-3);

  // Test the Backward function.
  module.Backward(input, target, output);

  CheckMatrices(output, arma::mat("-0.000000 0.166667 -1.500000 0.666667 "
      "0.000000 -0.000000"), 1e-3);
  BOOST_REQUIRE_EQUAL(output.n_rows, target.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, target.n_cols);

  // Test the error function on another input.
  input1 = arma::mat("0.4287 -1.6208 -1.5006 -0.4473 1.5208 -4.5184 9.3574 "
      "-4.8090 4.3455 5.2070");
  input2 = arma::mat("-4.5288 -9.2766 -0.5882 -5.6643 -6.0175 8.8506 3.4759 "
      "-9.4886 2.2755 8.4951");
  input = arma::join_cols(input1, input2);
  target = arma::mat("1 1 -1 1 -1 1 1 1 -1 1");
  error = module.Forward(input, target);
  BOOST_REQUIRE_CLOSE(error, 3.03530, 1e-3);

  // Test the Backward function on the second input.
  module.Backward(input, target, output);

  CheckMatrices(output, arma::mat("0.000000 0.000000 0.091240 0.000000 "
      "-0.753830 1.336900 0.000000 0.000000 -0.207000 0.328810"), 1e-6);
}

/**
 * Simple test for the Softmargin Loss function.
 */
BOOST_AUTO_TEST_CASE(SoftMarginLossTest)
{
  arma::mat input, target, output, expectedOutput;
  double loss;
  SoftMarginLoss<> module1;
  SoftMarginLoss<> module2(false);

  input = arma::mat("0.1778 0.0957 0.1397 0.1203 0.2403 0.1925 -0.2264 -0.3400 "
      "-0.3336");
  target = arma::mat("1 1 -1 1 -1 1 -1 1 1");
  input.reshape(3, 3);
  target.reshape(3, 3);

  // Test for sum reduction.

  // Calculated using torch.nn.SoftMarginLoss(reduction='sum').
  expectedOutput = arma::mat("-0.4557 -0.4761 0.5349 -0.4700 0.5598 -0.4520 "
      "0.4436 -0.5842 -0.5826");
  expectedOutput.reshape(3, 3);

  // Test the Forward function. Loss should be 6.41456.
  // Value calculated using torch.nn.SoftMarginLoss(reduction='sum').
  loss = module1.Forward(input, target);
  BOOST_REQUIRE_CLOSE(loss, 6.41456, 1e-3);

  // Test the Backward function.
  module1.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE(arma::as_scalar(arma::accu(output)), -1.48227, 1e-3);
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);
  CheckMatrices(output, expectedOutput, 0.1);

  // Test for mean reduction.

  // Calculated using torch.nn.SoftMarginLoss(reduction='mean').
  expectedOutput = arma::mat("-0.0506 -0.0529 0.0594 -0.0522 0.0622 -0.0502 "
      "0.0493 -0.0649 -0.0647");
  expectedOutput.reshape(3, 3);

  // Test the Forward function. Loss should be 0.712729.
  // Value calculated using torch.nn.SoftMarginLoss(reduction='mean').
  loss = module2.Forward(input, target);
  BOOST_REQUIRE_CLOSE(loss, 0.712729, 1e-3);

  // Test the Backward function.
  module2.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE(arma::as_scalar(arma::accu(output)), -0.164697, 1e-3);
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);
  CheckMatrices(output, expectedOutput, 0.1);
}
/**
 * Simple test for the Mean Absolute Percentage Error function.
 */
BOOST_AUTO_TEST_CASE(MeanAbsolutePercentageErrorTest)
{
  arma::mat input, target, output, expectedOutput;
  MeanAbsolutePercentageError<> module;

  input = arma::mat("3 -0.5 2 7");
  target = arma::mat("2.5 0.2 2 8");
  expectedOutput = arma::mat("10.0 -125.0 12.5 -3.125");

  // Test the Forward function. Loss should be 95.625.
  // Loss value calculated manually.
  double loss = module.Forward(input,target);
  BOOST_REQUIRE_CLOSE(loss, 95.625, 1e-1); 

  // Test the Backward function.
  module.Backward(input, target, output);
  BOOST_REQUIRE_CLOSE(arma::as_scalar(arma::accu(output)), -105.625, 1e-3);
  BOOST_REQUIRE_EQUAL(output.n_rows, input.n_rows);
  BOOST_REQUIRE_EQUAL(output.n_cols, input.n_cols);
  CheckMatrices(output, expectedOutput, 0.1);
}

BOOST_AUTO_TEST_SUITE_END();
