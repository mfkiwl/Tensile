/*******************************************************************************
 *
 * MIT License
 *
 * Copyright (C) 2019-2022 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *******************************************************************************/

#pragma once

#include <Tensile/ContractionProblem.hpp>
#include <Tensile/ContractionSolution.hpp>
#include <Tensile/Properties.hpp>

#include <cstddef>

namespace Tensile
{
    /**
     * \addtogroup Properties
     *
     * @brief An MLFeature is a Property whose value is of type `float`.
     *
     * This allows it to be used as an input to ML models.
     */
    namespace MLFeatures
    {
        /**
         * @brief A Property whose value is of type `float`.
         */
        template <typename Object>
        using MLFeature = Property<Object, float>;

        /**
         * \copydoc Tensile::Property_CRTP
         */
        template <typename Class, typename Object>
        using MLFeature_CRTP = Property_CRTP<Class, Object, float>;

        /**
         * \ingroup Properties
         * \defgroup MLFeatures MLFeature Classes
         *
         * @brief Individual MLFeature classes.
         */

        /**
         * \addtogroup MLFeatures
         * @{
         */
        struct FreeSizeA : public MLFeature_CRTP<FreeSizeA, ContractionProblem>
        {
            enum
            {
                HasIndex = true,
                HasValue = false
            };
            size_t index;

            static std::string Type()
            {
                return "FreeSizeA";
            }

            virtual float operator()(ContractionProblem const& problem) const
            {
                return (float)problem.freeSizeA(index);
            }
        };

        struct FreeSizeB : public MLFeature_CRTP<FreeSizeB, ContractionProblem>
        {
            enum
            {
                HasIndex = true,
                HasValue = false
            };
            size_t index;

            static std::string Type()
            {
                return "FreeSizeB";
            }

            virtual float operator()(ContractionProblem const& problem) const
            {
                return (float)problem.freeSizeB(index);
            }
        };

        struct BoundSize : public MLFeature_CRTP<BoundSize, ContractionProblem>
        {
            enum
            {
                HasIndex = true,
                HasValue = false
            };
            size_t index;

            static std::string Type()
            {
                return "BoundSize";
            }

            virtual float operator()(ContractionProblem const& problem) const
            {
                return (float)problem.boundSize(index);
            }
        };

        struct Tile0Granularity : public MLFeature_CRTP<Tile0Granularity, ContractionProblem>
        {
            enum
            {
                HasIndex = false,
                HasValue = true
            };
            float value; // 1/mt0

            static std::string Type()
            {
                return "Tile0Granularity";
            }

            virtual float operator()(ContractionProblem const& problem) const
            {
                float numTiles = problem.freeSizeA(0) * value;
                return ContractionSolution::computeGranularity(numTiles);
            }
        };

        struct Tile1Granularity : public MLFeature_CRTP<Tile1Granularity, ContractionProblem>
        {
            enum
            {
                HasIndex = false,
                HasValue = true
            };
            float value; // 1/mt1

            static std::string Type()
            {
                return "Tile1Granularity";
            }

            virtual float operator()(ContractionProblem const& problem) const
            {
                float numTiles = problem.freeSizeB(0) * value;
                return ContractionSolution::computeGranularity(numTiles);
            }
        };

        struct CUGranularity : public MLFeature_CRTP<CUGranularity, ContractionProblem>
        {
            enum
            {
                HasIndex = false,
                HasValue = true,

            };
            ContractionSolution::GranularityScaleFactors value;
            /* General scaling = (1 / (numCUs / globalSplitU / localSplitU))
             * See: `ContractionSolution::computeGranularities`
             */

            static std::string Type()
            {
                return "CUGranularity";
            }

            virtual float operator()(ContractionProblem const& problem) const
            {
                float NumBatches = 1; // TODO: Higher batch sizes
                float numTilesM  = problem.freeSizeA(0) * value.mt0_scale; // M / MT0
                float numTilesN  = problem.freeSizeB(0) * value.mt1_scale; // N / MT1
                float tilesPerCu
                    = NumBatches * ceil(numTilesM) * ceil(numTilesN) * value.devSolScale;

                return ContractionSolution::computeGranularity(tilesPerCu);
            }
        };

        struct WavesPerSIMD : public MLFeature_CRTP<WavesPerSIMD, ContractionProblem>
        {
            enum
            {
                HasIndex = false,
                HasValue = true,

            };
            ContractionSolution::GranularityScaleFactors value;
            /* General scaling = ((globalSplitU / numCUs)
             *                     * ceil((workGroupX * workGroupY) / wavefrontSize)
             *                     / (2 * simdPerCU))
             * See: `ContractionSolution::computeGranularities`
             */

            static std::string Type()
            {
                return "WavesPerSIMD";
            }

            virtual float operator()(ContractionProblem const& problem) const
            {
                float numTilesM  = problem.freeSizeA(0) * value.mt0_scale; // M / MT0
                float numTilesN  = problem.freeSizeB(0) * value.mt1_scale; // N / MT1
                float totalTiles = ceil(numTilesM) * ceil(numTilesN);
                return totalTiles * value.devSolScale;
            }
        };

        /**
         * @}
         */
    } // namespace MLFeatures
} // namespace Tensile
