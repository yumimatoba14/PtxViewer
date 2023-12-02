using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Tngn;

namespace PtxViewer
{
    class LengthDimensionMode : PickMode
    {
        private List<Tngn.Vector3d> pickedPoint = new List<Tngn.Vector3d>();

        public LengthDimensionMode() { }

        public LengthDimensionMode(Action onEndMode) : base(onEndMode) { }

        protected override void OnDetach(ViewModel viewModel)
        {
            Source.ClearLengthDimension();
            base.OnDetach(viewModel);
        }

        protected override bool OnPickingNone()
        {
            pickedPoint.Clear();
            return base.OnPickingNone();
        }

        protected override bool OnPickingPoint(Vector3d coord)
        {
            pickedPoint.Add(coord);
            if (1 < pickedPoint.Count)
            {
                Source.AddLengthDimension(pickedPoint[0], pickedPoint[1]);
                pickedPoint.Clear();
            }
            return base.OnPickingPoint(coord);
        }
    }
}
