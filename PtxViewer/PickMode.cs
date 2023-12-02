using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tngn;

namespace PtxViewer
{
    class PickMode : Tngn.ViewEventListener
    {
        public PickMode() { }

        public PickMode(Action onEndMode)
        {
            OnEndMode += onEndMode;
        }

        public event Action OnEndMode;

        public ViewModel Source;

        protected override void OnAttach(ViewModel viewModel)
        {
            Source = viewModel;
            viewModel.SetPickEnabled(true);
            base.OnAttach(viewModel);
        }

        protected override void OnDetach(ViewModel viewModel)
        {
            Action onEndMode = OnEndMode;
            if (onEndMode != null)
            {
                onEndMode();
            }
            viewModel.SetPickEnabled(false);
            base.OnDetach(viewModel);
        }
    }
}
