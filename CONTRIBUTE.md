# Contributing to Tasking #

The development in Tasking follows a feature branch workflow: create a feature branch from the *Master* branch, implement the code, rebase the branch on *Master* and then submit a merge request. GitLab ties this workflow together, in that it provides an issue tracker and handles the merge requests.

There is only one long live branch, the *Master*. Short-lived feature branches are used to develop features, fix bugs and make changes. The stable releases are merged on to the master branch. Before these can be merged back onto the master branch, they are tested by the Jenkins build server and reviewed by your colleagues. This ensures a continual improvement of the code quality.


## Feature Branch Workflow ##

The steps below give a more detailed view of the development process described above.

1. Create an issue in the relevant project on GitLab. It requires:
    * A title and a brief description of the problem
    * The assignee, due date, milestone and relevant labels
    
2. Create a feature branch from the issue and commit on it (let GitLab create the feature branch for you)
    
3. When ready, pull the master branch and rebase the feature branch on it
    
4. Push the commits on the feature branch to GitLab and create a merge request to:
    * Have Jenkins confirm that the tests pass
    * Allow colleagues to review your changes
    
5. Merge the changes into *Master* and delete the feature branch (let GitLab do the merge)

In order to ensure that the above workflow is adhered to by all and not accidentally infringed upon, the following rules have been setup on GitLab:
 
1. Protection of the master branch
    It is not possible to directly push onto the master branch. Any push should only be done on the feature branches. To incorporate your changes, you must use merge requests. For the merge requests to the *Master* branch, developer permissions are required. 

2. Merge request require that all relevant parties sign off
    * The Jenkins build is successful and the tests pass
    * Two colleagues are required to approve the merge request. Please select at least two appropriate and available colleagues when creating a merge request. @omai should always be one of the approvers. They may pose questions in the form of discussions pertaining to the relevant code. Only once all the discussions have been marked as resolved, will the merge request be accepted. Discussions should be resolved by the initiator of the discussion or a colleague who did not respond to the discussion item.


## Rebasing vs. Merging ##

We use rebases instead of merges (no dedicated merge commits). The main reason behind the choice is to avoid the mess left behind in the Git history. Another reason however, is that it forces the last person to create a merge request to handle the merge conflicts. These result from developing on feature branches in parallel, which is part of the supported workflow.


## Code Formatting ##

Follow the style being used in the repository you are working on (see Issue #38 for future improvement in this regard)! Everyone has a personal preference, every now and again the style guide endorsed by corporate changes, and sometimes it just depends on the weather. Therefore, to ensure that the code stays legible, keep with the style being employed in the surrounding code. Usually, a style formatting preset for your IDE is provided in the repository.


## Documentation Is Not Optional ##

It is not! Merge requests without documentation will be responded to with requests for clarification. This means:

 * The GitLab issue should include adequate description of what the problem is and which steps were taken to address it.
 
 * The code should include valid Doxygen comments for classes and functions as well as inline comments where necessary.


## Tests Are Not Optional ##

This is to ensure that bug fixes actually fix the bug without introducing unintended side-effects. In the case of features, they have to prove that they actually work. Therefore:

 * When adding features, the new code should be accompanied with adequate tests that cover the use cases. This should include edge cases and unexpected use.
 
 * When fixing bugs, the changes should include updates to the code as well as updating or adding tests that catch it.

It is strongly recommended to follow the test-driven development paradigm. Write your test first. It should should fail initially. Then implement the feature, bug fix, etc. until the test passes.

## References ##

Inspiration was taken from the following sources:

 * https://gitlab-ee.sc.dlr.de/scosa/ScOSA_System/blob/develop/CONTRIBUTE.md
 * http://www.contribution-guide.org/
 * https://www.atlassian.com/git/tutorials/comparing-workflows/centralized-workflow
 
