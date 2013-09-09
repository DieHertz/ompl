/* MorseEnvironment.h */

#ifndef OMPL_EXTENSION_MORSE_ENVIRONMENT_
#define OMPL_EXTENSION_MORSE_ENVIRONMENT_

#include "ompl/config.h"
#if OMPL_EXTENSION_MORSE == 0
#  error MORSE extension not built
#endif

#include "ompl/base/State.h"
#include "ompl/util/ClassForward.h"
#include "ompl/util/Console.h"

#include "boost/thread/mutex.hpp"

#include <limits>
#include <vector>

namespace ompl
{
    namespace base
    {

        /// @cond IGNORE
        /** \brief Forward declaration of ompl::base::MorseEnvironment */
        OMPL_CLASS_FORWARD(MorseEnvironment);
        /// @endcond

        /** \class ompl::base::MorseEnvironmentPtr
            \brief A boost shared pointer wrapper for ompl::base::MorseEnvironment */
        
        /** \brief This class contains the MORSE constructs OMPL needs to know about when planning. */
        class MorseEnvironment
        {
        public:
            
            /** \brief The dimension of the control space for this simulation */
            const unsigned int controlDim_;
            
            /** \brief Upper and lower bounds for each control dimension */
            const std::vector<double> controlBounds_;
            
            /** \brief The number of rigid bodies in the simulation */
            const unsigned int rigidBodies_;
            
            /** \brief Upper and lower bounds on position in each spatial dimension */
            std::vector<double> positionBounds_;
            
            /** \brief Upper and lower bounds on linear velocity in each spatial dimension */
            std::vector<double> linvelBounds_;
            
            /** \brief Upper and lower bounds on angular velocity in each spatial dimension */
            std::vector<double> angvelBounds_;
            
            /** \brief The simulation step size */
            double stepSize_;

            /** \brief The minimum number of times a control is applied in sequence */
            unsigned int minControlSteps_;

            /** \brief The maximum number of times a control is applied in sequence */
            unsigned int maxControlSteps_;
            
            /** \brief Indicates whether the simulation has been shut down externally */
            bool simRunning_;
            
            /** \brief Lock to use when performing simulations in the world */
            mutable boost::mutex mutex_;

            MorseEnvironment(const unsigned int controlDim, const std::vector<double> &controlBounds,
                const unsigned int rigidBodies, const std::vector<double> &positionBounds,
                const std::vector<double> &linvelBounds, const std::vector<double> &angvelBounds,
                const double stepSize, const unsigned int minControlSteps, const unsigned int maxControlSteps)
                 : controlDim_(controlDim), controlBounds_(controlBounds), rigidBodies_(rigidBodies),
                   positionBounds_(positionBounds), linvelBounds_(linvelBounds), angvelBounds_(angvelBounds),
                   stepSize_(stepSize), minControlSteps_(minControlSteps), maxControlSteps_(maxControlSteps),
                   simRunning_(true)
            {
                // Replace infinite bounds with very large bounds, so, e.g., sampling can still work
                for (unsigned int i = 0; i < positionBounds_.size(); i++)
                {
                    if (positionBounds_[i]==std::numeric_limits<double>::infinity())
                        positionBounds_[i] = std::numeric_limits<double>::max()/2;
                    else if (positionBounds_[i]==-std::numeric_limits<double>::infinity())
                        positionBounds_[i] = -std::numeric_limits<double>::max()/2;
                }
                for (unsigned int i = 0; i < linvelBounds_.size(); i++)
                {
                    if (linvelBounds_[i]==std::numeric_limits<double>::infinity())
                        linvelBounds_[i] = std::numeric_limits<double>::max()/2;
                    else if (linvelBounds_[i]==-std::numeric_limits<double>::infinity())
                        linvelBounds_[i] = -std::numeric_limits<double>::max()/2;
                }
                for (unsigned int i = 0; i < angvelBounds_.size(); i++)
                {
                    if (angvelBounds_[i]==std::numeric_limits<double>::infinity())
                        angvelBounds_[i] = std::numeric_limits<double>::max()/2;
                    else if (angvelBounds_[i]==-std::numeric_limits<double>::infinity())
                        angvelBounds_[i] = -std::numeric_limits<double>::max()/2;
                }
            }

            ~MorseEnvironment(void)
            {
            }

            /** \brief Get the control bounds -- the bounding box in which to sample controls */
            void getControlBounds(std::vector<double> &lower, std::vector<double> &upper) const;
            
            
            // These functions require interprocess communication and are left to be implemented in Python
            
            /** \brief Query the internal state of the simulation */
            virtual void readState(State *state) = 0;
            
            /** \brief Overwrite the internal state of the simulation */
            virtual void writeState(const State *state) = 0;
            
            /** \brief Configure simulation to proceed under a new control */
            virtual void applyControl(const std::vector<double> &control) = 0;
            
            /** \brief Proceed with the simulation for the given number of seconds */
            virtual void worldStep(const double dur) = 0;
        };
    }
}

#endif
