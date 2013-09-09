/* MorseSimpleSetup.cpp */

#include "ompl/extensions/morse/MorseSimpleSetup.h"
#include "ompl/extensions/morse/MorseProjection.h"
#include "ompl/extensions/morse/MorseTerminationCondition.h"
#include "ompl/util/Console.h"
#include <boost/thread.hpp>

ompl::control::MorseSimpleSetup::MorseSimpleSetup(const base::MorseEnvironmentPtr &env) :
    SimpleSetup(ControlSpacePtr(new MorseControlSpace(base::StateSpacePtr(new base::MorseStateSpace(env))))),
    env_(env)
{
    si_->setPropagationStepSize(env_->stepSize_);
    si_->setMinMaxControlDuration(env_->minControlSteps_, env_->maxControlSteps_);
    si_->setStatePropagator(StatePropagatorPtr(new MorseStatePropagator(si_)));
}

ompl::base::ScopedState<ompl::base::MorseStateSpace> ompl::control::MorseSimpleSetup::getCurrentState(void) const
{
    base::ScopedState<base::MorseStateSpace> current(getStateSpace());
    getStateSpace()->as<base::MorseStateSpace>()->readState(current.get());
    return current;
}

void ompl::control::MorseSimpleSetup::setCurrentState(const base::State *state)
{
    getStateSpace()->as<base::MorseStateSpace>()->writeState(state);
}

void ompl::control::MorseSimpleSetup::setCurrentState(const base::ScopedState<> &state)
{
    getStateSpace()->as<base::MorseStateSpace>()->writeState(state.get());
}

void ompl::control::MorseSimpleSetup::setup(void)
{
    if (!si_->getStateValidityChecker())
    {
        OMPL_INFORM("Using default state validity checker for MORSE");
        si_->setStateValidityChecker(base::StateValidityCheckerPtr(new base::MorseStateValidityChecker(si_)));
    }
    base::StateSpacePtr space = si_->getStateSpace();
    if (!space->hasDefaultProjection())
    {
        OMPL_INFORM("Registering MorseProjection as default projection evaluator for MORSE");
        space->registerDefaultProjection(base::ProjectionEvaluatorPtr(new base::MorseProjection(space)));
    }
    if (pdef_->getStartStateCount() == 0)
    {
        OMPL_INFORM("Using the initial state of MORSE as the starting state for the planner");
        pdef_->addStartState(getCurrentState());
    }
    SimpleSetup::setup();
}

ompl::base::PlannerStatus ompl::control::MorseSimpleSetup::solve(void)
{
    setup();
    return SimpleSetup::solve(base::MorseTerminationCondition(env_));
}

void ompl::control::MorseSimpleSetup::playSolutionPath(void) const
{
    if (haveSolutionPath())
        playPath(pdef_->getSolutionPath());
}

void ompl::control::MorseSimpleSetup::playPath(const base::PathPtr &path) const
{
    PathControl *pc = dynamic_cast<PathControl*>(path.get());
    if (pc)
    {
        unsigned int i;
        base::State *result = si_->allocState();
        for (i = 0; i < pc->getControlCount(); i++)
        {
            //double *con = pc->getControl(i)->as<RealVectorControlSpace::ControlType>()->values;
            //OMPL_INFORM("Applying control (%f,%f) for %f seconds", con[0], con[1], pc->getControlDuration(i));
            si_->getStatePropagator()->propagate(pc->getState(i), pc->getControl(i), pc->getControlDuration(i), result);
        }
        getStateSpace()->as<base::MorseStateSpace>()->writeState(pc->getState(i));
    }
    else
    {
        geometric::PathGeometric *pg = dynamic_cast<geometric::PathGeometric*>(path.get());
        if (!pg)
            throw Exception("Unknown type of path");
        if (pg->getStateCount() > 0)
        {
            OMPL_INFORM("Playing through %u states (%0.3f seconds)", (unsigned int)pg->getStateCount(),
                       si_->getPropagationStepSize() * (double)(pg->getStateCount() - 1));
            double d = si_->getPropagationStepSize();
            getStateSpace()->as<base::MorseStateSpace>()->writeState(pg->getState(0));
            for (unsigned int i = 1 ; i < pg->getStateCount() ; ++i)
            {
                getEnvironment()->worldStep(d);
                getStateSpace()->as<base::MorseStateSpace>()->writeState(pg->getState(i));
            }
        }
    }
}

ompl::base::PathPtr ompl::control::MorseSimpleSetup::simulateControl(const double* control, unsigned int steps) const
{
    Control *c = si_->allocControl();
    memcpy(c->as<MorseControlSpace::ControlType>()->values, control, sizeof(double) * getControlSpace()->getDimension());
    base::PathPtr path = simulateControl(c, steps);
    si_->freeControl(c);
    return path;
}

ompl::base::PathPtr ompl::control::MorseSimpleSetup::simulateControl(const Control* control, unsigned int steps) const
{
    PathControl *p = new PathControl(si_);

    base::State *s0 = si_->allocState();
    getStateSpace()->as<base::MorseStateSpace>()->readState(s0);
    p->getStates().push_back(s0);

    base::State *s1 = si_->allocState();
    si_->propagate(s0, control, steps, s1);
    p->getStates().push_back(s1);

    p->getControls().push_back(si_->cloneControl(control));
    p->getControlDurations().push_back(steps);
    return base::PathPtr(p);
}

ompl::base::PathPtr ompl::control::MorseSimpleSetup::simulate(unsigned int steps) const
{
    Control *c = si_->allocControl();
    si_->nullControl(c);
    base::PathPtr path = simulateControl(c, steps);
    si_->freeControl(c);
    return path;
}
